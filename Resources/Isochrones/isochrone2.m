set(0,'defaultTextInterpreter','latex');
set(groot, 'defaultAxesTickLabelInterpreter','latex'); 
set(groot, 'defaultLegendInterpreter','latex');
set(0,'defaultAxesFontSize',16)
files = "../../Downloads/test_" + ["low","mid","high"];

f = table();


for q = 1:length(files)
	figure(q);
	file = files(q);
	f = readtable(file,"NumHeaderLines",13);
	
	

f(f.label == 9,:) = [];
c = (isnan(f.MH));

tableStarts = [true;c(1:end-2)];
tableEnds = [c(2:end-1);true];

ns = 1:height(f);
startID = ns(tableStarts);
endID = ns(tableEnds);
clf; 
hold on;
zs = unique(f.MH);
zs(isnan(zs)) = [];
T = tiledlayout(3,3,'TileSpacing','Compact','Padding','Compact');
prev = 0;
inc = 5;
for i = 1:length(startID)
	
	prog = round(i/length(startID) * 100);
	if prog > prev + inc
		disp("Progress: " + num2str(prog) + "%");
		while prev < prog
			prev = prev + inc;
		end
	end
	
	subject = f(startID(i):endID(i),:);
	
	t = subject.logAge(1);
	z = subject.MH(1);
	MS = subject.Mini;
	on = ones(size(MS));
	zID = find(zs == z);
	nexttile(zID);
	hold on;
	scatter(MS,on*t,'b');
% 	
	hold off;
end
hold off;


% view(0,0)

for i = 1:length(zs)
	nexttile(i);
	title("[M/H] = " + num2str(zs(i)));
	xlabel("Mass");
	ylabel("Time");
	set(gca,'xscale','log');
	grid on;
end

end