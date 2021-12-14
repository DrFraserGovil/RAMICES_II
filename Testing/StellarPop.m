f = readtable("../Output/Ring/Ring0_stars.dat","ReadVariableNames",false,"NumHeaderLines",0);

ms = f{1,3:end};

empty = zeros(size(ms));
rs = empty;
time = 1;

clf;
T = tiledlayout(2,1);
nexttile;
ts = [];
mms = [];
for i = 2:height(f)/10
%     [i,f.Var1(i),time]
    if (f.Var1(i) == time)
       rs = rs + f{i,3:end};
       
    else
        hold on;
        plot3(empty + time,ms,log10(rs));
        
        hold off;
     
        ts(end+1) = time;
        mms(end+1) = sum(rs.*max(0.5,ms)/1e9);
        rs = f{i,3:end};
        time = time + 1;
    end
end
% hold on;
% plot3(empty + time,ms,rs);
% hold off;
% set(gca,'zscale','log');
view(3)
nexttile;
plot(ts,mms)