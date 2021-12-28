set(0,'defaultTextInterpreter','latex');
set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultAxesFontSize',16)
global kappa eta Tc Th Tigm coolFac Mc0 Ms0 Mh0
kappa = 0.3;
eta = 0.5;
Tc = 10;
Th = 1e5;
Tigm = 00;
coolFac = 1;
Mc0 = 10;
Ms0 = 0;
Mh0 = 0;
t0 = 0;
tf = 5;

tsHigh = linspace(t0,tf,100000);
tsLow = linspace(t0,tf,1000);

clf;
% [Mc,Mh,Ms] = bruteIntegrate(tsHigh);
% plotAll(tsHigh,Mc,Mh,Ms,'-');
[Mc,Mh,Ms] = iterative(tsLow);
plotAll(tsLow,Mc,Mh,Ms,'-');
[Mc,Mh,Ms] = exponentialCool(tsLow);
plotAll(tsLow,Mc,Mh,Ms,'--');



set(gca,'yscale','linear');
function plotAll(ts,Mc,Mh,Ms,style)
    lw = 2;
    subplot(2,2,[1,2]);
    hold on;
    plot(ts,Mc,'Color','b','LineStyle',style,'LineWidth',lw)
    plot(ts,Mh,'Color','r','LineStyle',style,'LineWidth',lw)
    plot(ts,Ms,'Color','y','LineStyle',style,'LineWidth',lw)
    plot(ts,Mc+Mh+Ms,'Color','k','LineStyle',style,'LineWidth',lw)
    hold off;
    
    subplot(2,2,3);
    hold on;
    plot(ts,Mh./(Mc + Mh));
    hold off;
    subplot(2,2,4);
    factor = 100;
    if (length(ts) > 1000)
        factor = 10;
    end
    tVH = linspace(ts(1),ts(end),factor*length(ts));
    [Mct,Mht,Mst] = bruteIntegrate(tVH);
    cdiff = zeros(size(ts));
    hdiff = cdiff;
    sdiff = cdiff;
    for i = 1:length(ts)
       [~,ii] = min(abs(tVH - ts(i)));
       
       cdiff(i) = (Mc(i) - Mct(ii));
       hdiff(i) = (Mh(i) - Mht(ii));
       sdiff(i) = (Ms(i) - Mst(ii));
    end
%     cdiff
    hold on;
    plot(ts,cdiff,'Color','b','LineStyle',style,'LineWidth',lw);
    plot(ts,hdiff,'Color','r','LineStyle',style,'LineWidth',lw);
    plot(ts,sdiff,'Color','y','LineStyle',style,'LineWidth',lw);
    hold off;
    
end

function [Mc,Mh,Ms] = bruteIntegrate(ts)
    global kappa eta Tc Th Tigm coolFac Mc0 Ms0 Mh0
    dt = ts(2) - ts(1);
    Mc = zeros(size(ts));
    Ms = Mc;
    Mh = Mc;
    Mc(1) = Mc0;
    Ms(1) = Ms0;
    Mh(1) = Mh0;
    for i = 2:length(ts)
        
        Msdot = kappa * Mc(i-1)^1.4;
        xi = Mh(i-1)/(Mh(i-1) + Mc(i-1));
%         Tav = Tc * (1-xi) + Th*xi
        xdot = coolFac * (Tigm - Tc)/(Th-Tc) - coolFac * xi;
        coolDot = (Mc(i-1) + Mh(i-1))*xdot;
%         [xi,xdot,Msdot, coolDot]
%         
        Ms(i) = Ms(i-1) + dt * Msdot;
        Mc(i) = Mc(i-1) - dt * (coolDot + (1 + eta)*Msdot);
        if (Mc(i) < 0)
            Mc(i) = 1e-8;
        end
        Mh(i) = Mh(i-1) + dt * (coolDot + eta * Msdot);
        if (Mh(i) < 0)
            Mh(i) = 1e-8;
        end
    end
end

function [Mc,Mh,Ms] = iterative(ts)
    global kappa eta Tc Th Tigm coolFac Mc0 Ms0 Mh0
    dt = ts(2) - ts(1);
    Mc = zeros(size(ts));
    Ms = Mc;
    Mh = Mc;
    Mc(1) = Mc0;
    Ms(1) = Ms0;
    Mh(1) = Mh0;
    n= 1.4;
    for i = 2:length(ts)
       Mc(i) = (Mc(i-1)^(1-n) + (n-1)*(1+eta)*kappa*dt)^(1/(1-n)); 
       Ms(i) = Ms(i-1) - 1/(1+eta)*(Mc(i) - Mc(i-1));
       Mh(i) = Mh(i-1) - eta/(1+eta) * (Mc(i) - Mc(i-1));
       
       tot = Mc(i) + Mh(i);
       xi = Mh(i)/tot;
       fac = (Tigm - Tc)/(Th - Tc);
       exper = exp(-coolFac *dt);
       xiEnd = (xi - fac)*exper + fac;
%        [Mc(i),Mh(i),fac,exper,xi,xiEnd]
       Mh(i) = xiEnd *tot;
       Mc(i) = (1-xiEnd)*tot;
%        
%        if ts(i) > 2
%            kappa = 0;
%        end
           
    end
    
end

function [Mc,Mh,Ms] = exponentialCool(ts)
    global kappa eta Tc Th Tigm coolFac Mc0 Ms0 Mh0
    dt = ts(2) - ts(1);
    Mc = zeros(size(ts));
    Ms = Mc;
    Mh = Mc;
    Mc(1) = Mc0;
    Ms(1) = Ms0;
    Mh(1) = Mh0;
    n= 1.4;
    for i = 2:length(ts)
       Mc(i) = (Mc(i-1)^(1-n) + (n-1)*(1+eta)*kappa*dt)^(1/(1-n)); 
       Ms(i) = Ms(i-1) - 1/(1+eta)*(Mc(i) - Mc(i-1));
       Mh(i) = Mh(i-1) - eta/(1+eta) * (Mc(i) - Mc(i-1));
       
       cooledMass = Mh(i) * (1-exp(-coolFac * dt));
       
       Mh(i) = Mh(i) - cooledMass;
       Mc(i) = Mc(i) + cooledMass;
    end
end