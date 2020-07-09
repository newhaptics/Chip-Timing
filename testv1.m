clear all
vid_dir = 'C:\Users\Alex\Documents\AmScope';
cd(vid_dir)
%%
filename = 'WIN_20200408_15_26_41_Pro';
video_ext = '.mp4';
% video = VideoReader([vid_dir '/' filename video_ext]);
video = VideoReader([filename video_ext]);


%%
% Label the areas of interest in order
name={'led','gate','in','out'};

% Number of areas of interest based on labels
outputs=length(name);

num_frames = video.NumberOfFrames;
start_frame=1;
end_frame=num_frames;

frame_rate = 16*video.framerate;
%%
% Read in frame 1 and show
this_frame = read(video,1);
this_frame= rgb2gray(this_frame); %convert to grayscale
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
close all;



%%
% load([vid_dir '/' filename '.mat']);
% Determine number of frames and define grayscalematrix and time
total_frames=end_frame-start_frame+1;

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
   this_frame=rgb2gray(this_frame);
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

%%
fig1=figure(1);
clf(fig1)
df = zeros(length(t),outputs);
for k=1:outputs
    df(:,k) = [0; diff(avgBright(:,k));];
    subplot(outputs,1,k);
    hold on;
    plot(t,avgBright(:,k)-min(avgBright(:,k)),'r-o','LineWidth',1.5);
    plot(t,df(:,k),'b-');
    set(gca,'fontsize', 16);
    grid on
    ylabel(name(k));
    ylim([0 max(avgBright(:,k))-min(avgBright(:,k))]);
    if k~=outputs
        set(gca,'XTick',[]);    
    end
    hold off;

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

save([vid_dir '/' filename '.mat']); % Save the data