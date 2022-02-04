set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);
files = "PadovaFiles/Met_" + ["m22_m18","m17_m13","m13_m09","m09_m05","m04_0","05_35","4_5"] + ".dat";
% files = "PadovaFiles/Met_" + ["m22_m18"];
ZCon = @(m) (1 - 0.2485)./(2.78 + 1/0.0207 * 10.^(-m));
f = table();

T = tiledlayout(1,1,'TileSpacing','None','Padding','None');
clf;
for i = 1:length(files)
	file = files(i);
	q = readtable(file,"NumHeaderLines",13);
	q.Properties.VariableNames= {q.Properties.VariableNames{2:end},'Error'};
	if height(f) > 0 
		zsPrev = unique(f.MH);
		zsNew = unique(q.MH);
		zCollide = intersect(zsPrev,zsNew);
		if ~isempty(zCollide)
			for z = zCollide
				q(q.MH == z,:) = [];
			end
		end
	end
	f = [f;q];
end
% f = [f;g;h];


f(f.label == 9,:) = [];
c = (isnan(f.MH));

tableStarts = [true;c(1:end-2)];
tableEnds = [c(2:end-1);true];



remainingMasses = f(tableEnds,:);

zs = unique(remainingMasses.MH);
zs(isnan(zs)) = [];
zzz = ZCon(zs);
Ms = 10.^linspace(log10(0.7),2,2000);

[Z,M] = meshgrid(zzz,Ms);
default = -1;
Tau = zeros(size(Z))+default;

zz = [];
mm = [];
tt = [];
zxSol = 0.0207;
for i = 1:length(zs)
	MH = zs(i)
	
    z = ZCon(MH);
	subject = remainingMasses(remainingMasses.MH == MH,:);
% 	subjectPostAGB = subject.label == 9;
% 	
% 	if sum(subjectPostAGB
% 	subject 
% 	q
% subject
	for j = 1:length(Ms)
		m = Ms(j);
		
		
		for k = 1:height(subject)-1
			
			down = subject.Mini(k);
			up = subject.Mini(k+1);
			downT = subject.logAge(k);
			upT = subject.logAge(k+1);
			if (m <= up && m > down) || (m > up && m <= down)
				fac = (m - down)/(up - down);
				predAge = downT + (upT-downT)*fac;
				Tau(j,i) = predAge;
				zz(end+1) = z;
				mm(end+1) = m;
				tt(end+1) = predAge;
			end
		end
	
	
		
	end
	
	
end
defaultOverride = 11;
Tau(Tau==default) = defaultOverride;


% scatter3(mm,zz,tt,10,zz);
% hold on;
% scatter3(mm,log10(10^10*(mm).^(-3)),zz);
% hold off;
hold on;
Zz = (Z/0.014);
% surf(M,Zz,Tau,'LineStyle','None');

imagesc([min(Ms),max(Ms)],[min(min(Zz)),max(max(Zz))],transpose(10.^Tau))
g = 0.4;
contour(M,Zz,Tau,[6:0.2:10.1],'LineColor',[g,g,g])
hold off;
xlabel("Mass ($M_\odot$)");
ylabel("Metallicity $(Z/Z_\odot)$");

xlim([min(Ms),max(Ms)]);
ylim([min(min(Zz)),max(max(Zz))])
set(gca,'yscale','log');
set(gca,'xscale','log');
set(gca,'colorscale','log');
% set(gca,'zscale','log');
caxis(10.^[6,10.3]);
c = colorbar;
c.TickLabelInterpreter = "latex";
c.Label.Interpreter = 'latex';
c.Label.String = "Age (Gyr)";

% view(0,0);
% view(2);

T2 = table();
c = 1;
for i = 1:length(Ms)
    for j = 1:length(zzz)
        T2(c,:) = {Ms(i), log10(zzz(j)),Tau(i,j)};
%         T2.LogZ(c) = zzz(j);
%         T2.Lifetime(c) = Tau(i,j);
        c = c + 1;
    end
end
T2.Properties.VariableNames = ["Mass","LogZ","LogLifetime"];
        
writetable(T2,"LifetimeGrid.dat")