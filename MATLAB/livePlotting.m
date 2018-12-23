% %% Create serial object for Arduino
% s = serial('COM9');
% %% Connect the serial port to Arduino
% s.InputBufferSize = 1; % read only one byte every time
% try
%     fopen(s);
% catch err
%     fclose(instrfind);
%     error('Make sure you select the correct COM Port where the Arduino is connected.');
% end

% %% Create a figure window to monitor the live data
% Tmax = 1000; % Total time for data collection (s)
% figure,
% grid on,
% xlabel ('Time (s)'), ylabel('Analog Value'),
% axis([0 Tmax+1 0 200]),
% %% Read and plot the data from Arduino
% Ts = 1; % Sampling time (s)
% i = 0;
% data = 0;
% t = 0;
% tic % Start timer
% while toc <= Tmax
%     i = i + 1;
%     %% Read buffer data
%     data(i) = fread(s);
%     %% Read time stamp
%     % If reading faster than sampling rate, force sampling time.
%     % If reading slower than sampling rate, nothing can be done. Consider
%     % decreasing the set sampling time Ts
%     t(i) = toc;
%     if i > 1
%         T = toc - t(i-1);
%         while T < Ts
%             T = toc - t(i-1);
%         end
%     end
%     t(i) = toc;
%     %% Plot live data
%     if i > 1
%         line([t(i-1) t(i)],[data(i-1) data(i)])
%         drawnow
%     end
% end
% fclose(s);

% clear a
% a = arduino()

fid = fopen('data.txt', 'w');

v = readVoltage(a, 'a0');
fprintf(fid, 'Voltage readings : %4.2f\n', v);

ii = 0;
Voltage = zeros(1e4, 1);

% figure, grid on,
% xlabel('Time'), ylabel('Voltage');
% axis([0 1000 0 200]),

