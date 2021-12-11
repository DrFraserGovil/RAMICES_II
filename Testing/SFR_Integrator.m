global R0 S0 sigma n nBelow delta;
R0 = 1;
delta = 0;
S0 = 10;
sigma = 1;
n = 2;
nBelow = 4;

TotalTime = 10;

T = tiledlayout(1,2,"TileSpacing","Compact","Padding","Compact");


nexttile(T);
s = linspace(0,S0,1000);
plot(s,sfr(s));
xlabel("Surface Density");
ylabel("SFR");

ts = linspace(0,TotalTime,1000);
nexttile(T);
plot(ts,manualIntegration(ts));
hold on;
plot(ts,oneToneIntegration(ts));
hold off;

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
    ss = (S0^pow - pow*(1+delta)*R0/sigma^nn * ts).^(1/pow);

end

function v = sfr(density)
    
    global R0 S0 sigma n nBelow
    
    below = (density < sigma) .* (density/sigma).^nBelow;
    above = (~below) .* (density/sigma).^n;
    
    v = R0 * (below + above);

end