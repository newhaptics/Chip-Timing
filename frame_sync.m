
stepT=1; % time between lamp steps, in msec
NumSteps=256; % number of steps, 0 to max value
cycleT = NumSteps*stepT; % time, in ms, for full period

figure (1)
subplot(2,1,1)
plot(avgBright,'ro-');

CamTicks=normalize(avgBright.^2.2,'range'); %  linearize (so intensity is proportional to time) and adjust to [0 1]
deltas=[diff(CamTicks);0]; % find local slopes and add one tick so vector lengths match
CamTicks(deltas<-0.05 & CamTicks >0.5 )=1; % round falling-edge high ticks to 1
CamTicks(deltas<-0.05 & CamTicks <= 0.5)=0; % round falling-edge lo ticks to 0

subplot(2,1,2)
RealTicks=unwrap(CamTicks*2*pi)/(2*pi); % "unwrap" time by adding 1 unitless period every time signal resets
RealTicks=RealTicks * cycleT; % convert unitless periods to absolute time, in msecm, for each frame
plot(RealTicks,CamTicks,'o-')
elapsedT=RealTicks(end)-RealTicks(1);
title(strcat('Elapsed time =',num2str(elapsedT),'ms'));







