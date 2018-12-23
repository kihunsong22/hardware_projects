% create an arduino object

a = arduino('com3', 'uno');     

% calculate a brightness step

brightness_step = (5-0)/20;

% start a loop to brighten up the led

for i = 1:20

    writePWMVoltage(a, 'D11', i*brightness_step);

    pause(0.1)

end

% start a loop to turn off the led in steps so that it looks like 

% led is fading

for i = 1:20;

    writePWMVoltage(a, 'D11', 5-i*brightness_step);

    pause(0.1)

end

% end communication with arduino

clear a