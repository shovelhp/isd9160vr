%function plotadcdata(filename)
%filename = 'recordtest.log';
mkdir('wav');
bindir = 'data\';
wavdir = 'wav\';
% dir = '';
filename = 'record600s1_201902181433';
binfile = [bindir filename '.bin'];
wavfile = [wavdir filename '.wav'];
fs = 8000;
file = fopen(binfile, 'r');
readtime = 1000;
readsize = 1000+8000*2*readtime;
%data = fread(file,readsize,'uint8');
data = fread(file,'uint8');
% datahex = dec2hex(data);
datachar = char(data);
datastr = string(datachar');
datas16 = [];
frameheader = char([hex2dec('55') hex2dec('aa') hex2dec('aa') hex2dec('55') hex2dec('55') hex2dec('aa') hex2dec('aa') hex2dec('55')]);
%frameheader = char([hex2dec('55') hex2dec('aa') hex2dec('aa') hex2dec('55')]);
%frameheader = char([hex2dec('55') hex2dec('aa')]);
frameheadersize = length(frameheader);
index = strfind(datastr, frameheader);
for secindex = 1:length(index)-1
    datasec = data(index(secindex)+frameheadersize+2:index(secindex+1)-1);
    if mod(length(datasec),2)
        datasec = [datasec;datasec(length(datasec))];
    end
    datasec2 = reshape(datasec,2,[])';
    datasec16 = (datasec2(:,1)*256)+datasec2(:,2);
%     datasechex16 = dec2hex(datasec16);
    datasecu16 = uint16(datasec16);
    datasecs16 = typecast(datasecu16, 'int16');
    %plot(datasecs16);
    datas16 = [datas16;datasecs16];
end

audiowrite(wavfile, datas16, fs);

% f1 = figure;
% p1 = plot(datas16);
% l = length(datas16)+8000;
% t = l/8000;
% disp(t);
% xlim([0 length(datas16)+8000]);
% ylim([-32768 32768]);
% xticks(0:8000:l);
% xtmax = floor(t);
% xtstep = floor(xtmax/10);
% xt = 0:xtstep:floor(t);
% xtt = 0:xtstep*8000:l;
% xticks(xtt);
% xts = num2str(xt');
% xtc = cellstr(xts)';
% xticklabels(xtc);