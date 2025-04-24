sret = "Set M";
file_name = strcat(sret,"/tab_yieldstot_ele_exp.dec");
fd = fopen(file_name);
S = textscan(fd,'%s','delimiter','\t') ;
S = S{1};
t = cell(0,0);
nCompleted = 0;
fast = cell(0,0);
medium = cell(0,0);
slow = cell(0,0);
for i = 1:length(S)
    r = textscan(S{i},'%s','delimiter',' ');
    r = r{1};
    
    if strcmp(r{1}, 'ele')
        speed = mod(nCompleted-1,3);
        if nCompleted > 0    
            switch speed
                case 0
                    slow{end+1} = t;
                case 1
                    medium{end+1} = t;
                case 2
                    fast{end+1} = t;
            end
        end
        
        
        nCompleted = nCompleted + 1;
        t = cell(0,0);
        height = 1;
    end
    q = 1;
    for j = 1:length(r)
        if ~isempty(r{j})
            t{height,q} = r{j};
            q = q+1;
        end
    end
    height = height + 1;
end
fast{end+1} = t;
slow{1}
slow{2}
allYields = {slow, medium, fast};
masses = [13,15,20,25,30,40,60,80,120];
names = {"slow", "medium", "fast"};
wantedZ = [1,2,0,26,8,12,6,14,20,25,24,27,10];
for speed = 1:3
    fileName = strcat(sret,"/rotYields_",names{speed},".dat");
    fileID = fopen(fileName,'w');
    for mIndex = 1:length(masses)
        for zIndex = length(allYields{speed}):-1:1
            relevant = allYields{speed}{zIndex};
            initialTotal = sum(str2num(cell2mat(relevant(2:end,4))));
            X = str2num(relevant{2,4})/initialTotal;
            Y = str2num(relevant{3,4})/initialTotal;
            Z = 1 - X - Y;
            
            totalOutput = sum(str2num(cell2mat(relevant(2:end,4+mIndex))));
            remnantMass = masses(mIndex) - totalOutput;
            
            yields = [];
            for i = [1,2,4:length(wantedZ)]
                protons = wantedZ(i);
                yields(i) = str2num(relevant{1+protons,4+mIndex});
            end
            yields(3) = totalOutput - yields(1) - yields(2);
            yields(13) = 0;
            line = [];
            line = strcat(num2str(masses(mIndex)),'\t',num2str(Z),'\t',num2str(remnantMass));
            for i = 1:length(yields)
                line = strcat(line,'\t',num2str(yields(i)));
            end
            line;
            line = strcat(line,'\t <-Yields/Original-> \t',num2str(X),'\t',num2str(Y),'\t',num2str(Z));
       
            for i = 4:length(yields)
                protons = wantedZ(i);
                p = relevant{1+protons,4};
                if i == length(yields)
                   p = 0; 
                end
                line = strcat(line,'\t',num2str(p));
            end
            
            
            line = strcat(line,'\n');
            line;
            fprintf(fileID,line);
        end
    end
    fclose(fileID);
end







