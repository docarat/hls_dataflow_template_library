clear;

graphics_toolkit("gnuplot");
load ../data/data.dat;
load derivative_task.dat

figure 1
subplot(1,2,1);plot(data);
title('Input stream 10-bit ADC')
subplot(1,2,2);plot(derivative_task);
title('1st derivative');


