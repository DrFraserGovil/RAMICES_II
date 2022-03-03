set(groot, 'defaultAxesTickLabelInterpreter','latex'); set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultTextInterpreter','latex');
set(0,'defaultAxesFontSize',28);
% files = "PadovaFiles/Met_" + ["m22_m18","m17_m13","m13_m09","m09_m05","m04_0","05_35","4_5"] + ".dat";
files = "PadovaFiles/Met_" + ["m22_m18"] + ".dat";
% files = files(2)

f= parseTable(files);


ts = unique(f.logAge);
ts(isnan(ts)) = [];
zs = unique(f.Zini);
zs(isnan(zs)) = [];

figure(3);
clf;

closest = [];
vals = 10.^[0.02,0.04,0.06];
for i = 1:length(vals)
    d = abs(ts - vals(i));
    closest(i) = find(d == min(d));
end


MS = linspace(0.1,100,1000);
zID = ceil(length(zs)/2);

V = jet(length(MS));

mColor = zeros(size(MS));
mMag = mColor;
mTeff = mMag;
mLum = mMag;
colLab = mMag;
for i = 1:length(ts)
   t= ts(i);
    for m = 1:length(MS)
         mass = MS(m);
    %    for j = round(linspace(1,length(zs),8))
           z = zs(zID);

           subselect = (f.logAge == t) & (f.Zini == z);
           
           sample = f(subselect,:);
           
           mD = abs(sample.Mini - mass);
           [~,I] = min(mD);
           
           
           
           color = sample.Bmag - sample.Vmag;
           mag = sample.Vmag;
           teff = (sample.logTe);
           lum = sample.logL;
           
%            drawnow;
           mColor(m) = color(I);
           mMag(m) = mag(I);
           mTeff(m) = teff(I);
           mLum(m) = lum(I);
           colLab(m) = sample.label(I);
    end
    subplot(3,1,1);
    hold on;
    plot(MS,mMag);
    subplot(3,1,2);
    hold on;
    scatter(mColor,mMag,5,colLab);
    hold off;
    set(gca,'ydir','reverse');

    drawnow;

    subplot(3,1,3);
    hold on;
    scatter(mLum, mTeff,5,colLab);
    hold off;
    colorbar;
    caxis([min(colLab),max(colLab)])
%     [mColor; mTeff]
    %    end
end


function f= parseTable(files)
f = table();
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
f(f.label > 8,:) = [];
end