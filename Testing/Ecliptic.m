alphas = 0:1:360;
delta = -10;

figure(1);
clf;
ls = [];
bs = [];
for i = 1:length(alphas)
    alpha = alphas(i);
    subplot(2,1,1);
    hold on;
    scatter(alpha,delta);
    hold off;
    [l,b] = transform(alpha,delta);
    subplot(2,1,2);
    hold on;
    ls(end+1) = l;
    bs(end+1) = tand(-b);
    scatter(l,(-b));
    hold off;
    drawnow;
end

C = [-1.241,-0.1148];
D = [1.916,-0.255];
ll = linspace(-200,200,1000);
vs = ll - ll + 0.7146;
for i = 1:length(C)
   vs = vs + D(i) * sind(i * ll) + C(i) * cosd(i*ll); 
end
hold on;
plot(ll,atand(vs));
hold off;
grid on;

function [l,b] = transform(alpha,delta)
    delta0 = 27.1282;
    l0  = 32.93;
    alpha0 = 282.25;

    sinb = sind(delta) * sind(delta0) - cosd(delta) * cosd(delta0) * sind(alpha - alpha0);
    
    b = asind(sinb);
    cosb = sqrt(1 - sinb^2);
    sinl = (sind(delta) * cosd(delta0) + cosd(delta) * sind(delta0) * sind(alpha - alpha0) ) / cosb;
    cosl = cosd(alpha - alpha0) * cosd(delta)/cosb;
    l = atan2d(sinl,cosl) + l0;
end