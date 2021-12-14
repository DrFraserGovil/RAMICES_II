global R0 S0 sigma n nBelow delta lambda;
R0 = 1;
delta = 0.6;
lambda = 1;
S0 = 10;
sigma = 1e-6;
n = 1.4;
nBelow = 4;

TotalTime = 12;

T = tiledlayout(1,2,"TileSpacing","Compact","Padding","Compact");


nexttile(T);
s = linspace(0,S0,1000);
plot(s,sfr(s));
xlabel("Surface Density");
ylabel("SFR");

ts = linspace(0,TotalTime,20000);
tsRough = linspace(0,TotalTime,200);
nexttile(T);
[c,h] = dumbIntegral(ts,10,0);
plot(ts,c);
hold on;
plot(ts,h);
[c,h] = dumbIntegral(tsRough,10,0);
plot(tsRough,c);
plot(tsRough,h);
[c,h] = CGDecay(tsRough,10,0);
plot(tsRough,c);
plot(tsRough,h);
% plot(tsRough,semiClever(tsRough,10,0))
hold off;
xlabel("Time (Gyr)");
ylabel("Mass $(10^{10} M_\odot)$");
% set(gca,'yscale','log')
legend("Cold Gas (Brute Force)","Hot Gas (Brute Force)","Cold Gas (Bas)","Hot Gas (Bas)","Cold Gas (CG Decay)","Hot Gas (CG Decay)","Cold Gas (iterated)","Hot Gas (iterated)")
function ss = manualIntegration(ts)
   global S0 delta
   ss = zeros(size(ts));
   ss(1) = S0;
   for i = 2:length(ts)
      dt = ts(i) - ts(i-1);
      rho = sfr(ss(i-1));
      ss(i) = ss(i-1) - (1+delta)*rho*dt;
   end
end

function ss = oneToneIntegration(ts)
    global R0 S0 sigma n nBelow delta;
    
    nn = n;
    if S0 < sigma
        nn = nBelow;
    end
    pow = 1-nn;
    ss = (S0^pow - pow*(1+delta)*R0* ts).^(1/pow);

end
function [c,h] = semiClever(ts,initCold,initHot)
    global S0 lambda
     c = zeros(size(ts));
    h = c;
    c(1) = initCold;
    h(1) = initHot;
    for i = 2:length(ts)
       [dc,dh] = derivative(c(i-1),h(i-1));
       dt = (ts(i)-ts(i-1));
%        c(i) = c(i-1) + dc*dt;
       h(i) = h(i-1) + dh*dt;
       S0 = c(i-1);
       c(i) = oneToneIntegration(dt) + lambda*h(i-1)*dt;
    end
end
function [c,h] = CGDecay(ts,initCold,initHot)
    global S0 lambda delta
     c = zeros(size(ts));
    h = c;
    c(1) = initCold;
    h(1) = initHot;
    for i = 2:length(ts)
        S0 = c(i-1);
       newC = oneToneIntegration(ts(i) -ts(i-1));
       loss = S0 - newC;
       heat = delta/(1 + delta)*loss;
       
       startHot = h(i-1) + heat;
       endHot = startHot * exp(-lambda * (ts(i) - ts(i-1)));
       cool = startHot-endHot;
       c(i) = newC + cool;
       h(i) = endHot;
    end
end

function [c,h] = dumbIntegral(ts,initCold,initHot)
    c = zeros(size(ts));
    h = c;
    c(1) = initCold;
    h(1) = initHot;
    for i = 2:length(ts)
       [dc,dh] = derivative(c(i-1),h(i-1));
       dt = (ts(i)-ts(i-1));
       c(i) = c(i-1) + dc*dt;
       h(i) = h(i-1) + dh*dt;
    end
end
function [dCold,dHot] = derivative(cold,hot)
    global delta lambda
    s = sfr(cold);
    dCold = -(1+delta)*s + lambda * hot;
    dHot = delta * s - lambda * hot;
end
function v = sfr(density)
    
    global R0 S0 sigma n nBelow
    
    below = (density < sigma) .* (density).^nBelow;
    above = (~below) .* (density).^n;
    L0 = R0 * sigma^(n - nBelow);
    v = L0 * below + R0 * above;

end