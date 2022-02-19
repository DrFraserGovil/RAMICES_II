set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);

files = "../Output/Pollution/" + ["Dormant"] + "/Enrichment_Absolute_ColdGas.dat";
% files = "../Output/Pollute_" + ["NoStars","RadialFlow_NoIGM","RadialFlow"] + "/Enrichment_Absolute_ColdGas.dat";
names = ["No Star Formation","Radial Inflow, No IGM","Radial Inflow"];
times = round([2.^linspace(log2(0.1),log2(40),9)],2,'significant');

figure(1);
clf;
T = tiledlayout('flow');
for file = files
    file
    track(file,times)
end


nexttile(length(times));
legend(names);

function track(fileName,times)
    opts = detectImportOptions(fileName);

    opts.VariableTypes(:) = {'double'};

    f = readtable(fileName,opts);

    simTimes = unique(f.Time);
    tSim = [];
	maxVal = 0;
    for i = 1:length(times)
        t = times(i);
        [~,I] = min(abs(simTimes - t));
        tSim(end+1) = I;
%         simTimes(I)
%         [t,I,simTimes(I)]
        nexttile(i);
        cut = (f.Time == simTimes(I));    
        focus = f(cut,:);
%         focus(1:3,:)
        
        hold on;
        v = focus.Total_Eu./focus.Total_H;
% 		if i == 1
			maxVal = max(focus.Total_H);
%             maxH = max(focus.Total_H);
%         end
        s = sum(focus.Total_Eu);
        plot(focus.RingRadius,v/maxVal);
%         plot(focus.RingRadius,focus.Total_H/maxVal);
        hold off;
        set(gca,'yscale','log')
        xlim([0,20]);
        ylim([1e-20,1]);
        sum(focus.Total_Eu)
        grid on;
        title("$t = " + num2str(t) + "$");
        
       
    end
    

end