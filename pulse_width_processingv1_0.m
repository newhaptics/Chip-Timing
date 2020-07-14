%clear previous program and enter the required directories
vid_dir = 'C:\Users\alex\Documents\FC_videos';
RAM_dir = 'R:\';
file_dir = 'C:\Users\alex\Dropbox\Chip Timing';

cd(vid_dir)
addpath(RAM_dir)  
addpath(file_dir)

save_data = 1;  % 0 to turn off save-data
%% take in a video file and create an object
filename = 'S5_M3AR_1_E0_r1_c1_in1_s-4_h40_p30_E1_r2_c1_in1_s-4_h40_p30_E2_r3_c1_in1_s-4_h40_p30_E3_r4_c1_in1_s-4_h40_p30_E4_r5_c1_in1_s-4_h40_p30__20200713T152806';
video_ext = '.avi';

if isfile([RAM_dir '/' filename video_ext])
    copyfile([RAM_dir '/' filename video_ext]);
    delete([RAM_dir '/' filename video_ext]);
end

video = VideoReader([vid_dir '/' filename video_ext]);

%%
% Label the areas of interest in order
% name={'led_timer','gate_led','gate','in','out'};
name = {'gate LED', 'col 1', 'col 2', 'row 1', 'row 2', 'row 3', 'row 4', 'row 5', 'out 1 1', 'out 2 1', 'out 3 1', 'out 4 1', 'out 5 1', 'out 1 1', 'out 1 2', 'out 1 3', 'out 1 4', 'out 1 5'};


% Number of areas of interest based on labels
outputs=length(name);

num_frames = video.NumberOfFrames;
start_frame=1;
end_frame=num_frames;

frame_rate = 16*video.framerate;
%% Aquire the position of croping matrix

reply='';
while strcmp(reply,'y')==0 && strcmp(reply,'n')==0
    reply=input('Do you want to pick new crop regions? y/n (yes/specify path) : ','s');
end

if strcmp(reply,'y')
    
    % Read in frame 1 and show
    this_frame = read(video,1);
    this_frame = rgb2gray(this_frame); %convert to grayscale
    thisfig = figure();
    imshow(this_frame);

    posMatrix=zeros(outputs,4);
    % posMatrixCrops=zeros(outputs,4,5); % max of five interior crop regions
    % num_crop_regions=zeros(length(outputs),1);
    thisfig = figure(1);

    for i=1:outputs
        clear reg
        close(thisfig)
        thisfig = figure(1);
        imshow(this_frame);
        set(gcf, 'units', 'normalized', 'position', [0 0 1 1]);
        disp(['For region: ' name(i)]);

        
        [reg,posMatrix(i,:)]= imcrop(this_frame);

    %% For making multiple crop regions per area of interest
    %     num_crop_regions(i,1)=input('Enter the number of cropped regions: ');
    % 
    %     for j=1:num_crop_regions(i,1)
    %         clear crop
    %         close(thisfig)
    %         thisfig = figure(1);
    %         imshow(reg);
    %         [crop,posMatrixCrops(i,:,j)]= imcrop(reg);
    %     end

    end
    save([vid_dir '/' 'posMatrix_temp' '.mat'],'posMatrix');
    close all;

elseif strcmp(reply,'n')
    load_file=input('Specify file name: ','s');
    load([vid_dir '/' load_file '.mat'],'posMatrix');
end



%% Acquire the average brightness matrix
% load([vid_dir '/' filename '.mat']);
% Determine number of frames and define grayscalematrix and time
total_frames=end_frame-start_frame;

t=zeros(total_frames,1);
% grayscaleValMat = zeros(total_frames,outputs);

% Define brightness matrix
avgBright = zeros(total_frames,outputs);

% % Iterate through frames calculate average brigtness for each region
% x=0; % create variable to track progress of program


tic
for i = 1:total_frames

   currentTime = (i-1)*(1/frame_rate);
   this_frame = read(video,start_frame-1+i);
   this_frame = rgb2gray(this_frame);
   t(i,1) = currentTime;
%    pause(.001);
   for j=1:outputs
       zoom_frame=imcrop(this_frame,posMatrix(j,:));
       sum_pixels=0;
       num_pixels=0;
%        if num_crop_regions(j,1) ~=1
%            for k=1:num_crop_regions(j,1)
%                sum_pixels=sum_pixels+sum(sum(imcrop(zoom_frame,posMatrixCrops(j,:,k))));
%                num_pixels=num_pixels+numel(imcrop(zoom_frame,posMatrixCrops(j,:,k)));
%            end
%            avgBright(i,j)=sum_pixels/num_pixels;
%        else
           avgBright(i,j)=mean2(imcrop(this_frame,posMatrix(j,:)));
%        end

%        maxBright(j)=max(max(imcrop(this_frame,posMatrix(j,:))));
%        minBright(j)=max(max(imcrop(this_frame,posMatrix(j,:))));
%        grayscaleValMat(i,j)=avgBright(j);
%        grayscaleValMax(i,j)=maxBright(j);
%        grayscaleValMin(i,j)=minBright(j);
   end
end
toc

%% aquire the differential matrix 
fig1=figure(1);
clf(fig1)
df = zeros(length(t),outputs);
for k=1:outputs
    df(:,k) = [0; diff(avgBright(:,k));];
    subplot(outputs,1,k);
    hold on;
    plot(t,avgBright(:,k)-min(avgBright(:,k)),'r','LineWidth',1.5);
%     plot(t,df(:,k),'b-');
    set(gca,'fontsize', 16);
    grid on
    ylbl = ylabel(name(k));
    ylim([0 max(avgBright(:,k))-min(avgBright(:,k))]);
    xlim([.15 .5])
    if  k~=outputs
        set(gca,'XTick',[]);    
    end
    hold off;
    set(get(gca,'YLabel'),'Rotation',360,'HorizontalAlignment','right');
    if k==outputs
        xlabel('Time (s)');
    end
end
%%

% count=0;
% for i=1:length(t)
%     if df(i)>4 
%         count = count +1;
%     end
% end
% count

%%
if save_data==1
    save([vid_dir '/' filename '-' datestr(now) '.mat']); % Save the data
    save([file_dir '/' filename '-' datestr(now) '.mat']); % Save the data
end