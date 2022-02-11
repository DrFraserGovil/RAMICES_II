set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);

files = "../Output/Pollute_" + ["RadialFlow_NoIGM"] + "/Enrichment_Absolute_ColdGas.dat";
% files = "../Output/Pollute_" + ["NoStars","RadialFlow_NoIGM","RadialFlow"] + "/Enrichment_Absolute_ColdGas.dat";
names = ["No Star Formation","Radial Inflow, No IGM","Radial Inflow"];
times = round([2.^linspace(log2(0.1),log2(24.2),9)],2,'significant');

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
		if i == 1
			maxVal = max(focus.Total_Eu)
		end
        plot(focus.RingRadius,focus.Total_Eu/maxVal);
        hold off;
        set(gca,'yscale','linear')
        xlim([0,20]);
        ylim([1e-7,1]);
        sum(focus.Total_Eu)
        grid on;
        title("$t = " + num2str(t) + "$");
        
       
    end
    

end