clear;

graphics_toolkit("gnuplot");
load ../data/data.dat;
load result01.dat
load result02.dat
load test_data.dat

figure 1
subplot(1,2,1);plot(data);
title('Input stream float')
subplot(1,2,2);plot(result01);
title('Smoothed output stream float');
figure 2
subplot(1,2,1);plot(test_data);
title('Input stream uint10');
subplot(1,2,2);plot(result02);
title('Smoothed output stream uint10');


