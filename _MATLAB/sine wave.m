%% basic example
% x = [0:0.1:10*pi]
% y = sin(x)
% plot(x, y)

% %% Time specifications:
% Fs = 800;                    % samples per second
% dt = 1/Fs;                   % seconds per sample
% StopTime = 1;                % seconds
% x = (0:dt:StopTime-dt)';     % seconds

% %% Sine wave:
% Fc = 60;                     % hertz
% y = sin(2*pi)
% % y = cos(2*pi*Fc*x);
% % Plot the signal versus time:
% figure;
% plot(x,y);
% xlabel('time (in seconds)');
% ylabel('SINE')
% title('Signal versus Time');
% zoom xon;


fs = 128; % Sampling frequency (samples per second) 
 dt = 1/fs; % seconds per sample 
 StopTime = 6; % seconds 
 t = (0:dt:StopTime)'; % seconds 
 F = 60; % Sine wave frequency (hertz) 
 data = sin(2*pi*F*t);
 plot(t,data)
 %% For one cycle get time period
 T = 1/F ;
 % time step for one time period 
 tt = 0:dt:T+dt ;
 d = sin(2*pi*F*tt) ;
 plot(tt,d) ;