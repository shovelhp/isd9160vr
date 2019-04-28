classdef VoiceRecordnl_exported < matlab.apps.AppBase

    % Properties that correspond to app components
    properties (Access = public)
        ISD9160VoiceRecordUIFigure  matlab.ui.Figure
        TabGroup                    matlab.ui.container.TabGroup
        VoiceRecordTab              matlab.ui.container.Tab
        VoiceRecordPanel            matlab.ui.container.Panel
        RecordSW                    matlab.ui.control.Button
        UARTEditFieldLabel          matlab.ui.control.Label
        UARTPort                    matlab.ui.control.EditField
        EditField_2Label            matlab.ui.control.Label
        RecordLen                   matlab.ui.control.NumericEditField
        EditField_3Label            matlab.ui.control.Label
        FrameSize                   matlab.ui.control.NumericEditField
        EditField_4Label            matlab.ui.control.Label
        FilenamePrefix              matlab.ui.control.EditField
        EditFieldLabel              matlab.ui.control.Label
        RecordTimes                 matlab.ui.control.NumericEditField
        RecordStatus                matlab.ui.control.Lamp
        Label                       matlab.ui.control.Label
        RecordBaud                  matlab.ui.control.NumericEditField
        Label_2                     matlab.ui.control.Label
        DispWindow                  matlab.ui.control.TextArea
        DispData                    matlab.ui.control.CheckBox
        FileDisplayTab              matlab.ui.container.Tab
        FileDisplayPanel            matlab.ui.container.Panel
        DispSW                      matlab.ui.control.Button
        Label_3                     matlab.ui.control.Label
        DispFilename                matlab.ui.control.EditField
        DispStatus                  matlab.ui.control.Lamp
        DispAxes                    matlab.ui.control.UIAxes
        Bin2WavSW                   matlab.ui.control.Button
        TransStatus                 matlab.ui.control.Lamp
    end

    
    properties (Access = private)
        RecRuning = 0;% Description
        RecIndex = 1;
        DataDirName = 'data' % Description
        WavDirName = 'wav'% Description
    end
    
    methods (Access = private)
        
        function RecStart(app,sComPort,RecTimes,RecLen,RecFrameSize,RecFilenamePre,RecBaud)
            sComPort.InputBufferSize = RecBaud*10;
            %sComPort.BytesAvailableFcnCount = 1;
            fopen(sComPort);
            RecStatus = strcmp(sComPort.Status, 'open');
            if(~RecStatus)
                app.RecIndex = 1;
                app.RecRuning = -1;
                return
            end
            app.RecIndex = 1;
            app.RecRuning = 1;
            app.RecordStatus.Color = 'r';
            app.RecordSW.Text = 'ֹͣ�ɼ�';
            pause(0.1);
            filetimestr = datestr(now,'yyyymmddHHMM');
            filedirstatus = mkdir(app.DataDirName);
            if(~filedirstatus)
                app.RecIndex = 1;
                app.RecRuning = -1;
                return
            end
            for index = 1:RecTimes
                app.RecIndex = index;
                recfilename = ['.\' app.DataDirName '\' RecFilenamePre num2str(index) '_' filetimestr '.bin' ];
                recfilehn = fopen(recfilename,'w');
                tic;
                runningtime = toc;
                while(runningtime <= RecLen)
                    pause(0.5);
                    bytercv = sComPort.BytesAvailable;
                    data = fread(sComPort, bytercv, 'uint8');
                    if(~isempty(data))
                        fwstatus = fwrite(recfilehn, data, 'uint8');
                    end
                    if(~fwstatus)
                        app.RecRuning = -1;
                        fclose(recfilehn);
                        break
                    end
                    if(app.DispData.Value)
                        app.DispWindow.Value = string(char(data)');
                    end
                    %pause(0.5);
                    runningtime = toc;
                end
                fclose(recfilehn);
            end
            fclose(sComPort);
            app.RecRuning = 0;
            app.RecordStatus.Color = 'g';
            app.RecordSW.Text = '��ʼ�ɼ�';
            pause(0.1);
        end
        
        function RecStop(app,RecTimes,sComPort)
            fclose(sComPort);
            RecStatus = strcmp(sComPort.Status, 'closed');
            if(~RecStatus)
                app.RecIndex = 1;
                app.RecRuning = -1;
                return
            end
            %pause(1);
            app.RecRuning = 0;
            app.RecIndex = RecTimes;
            app.RecordStatus.Color = 'g';
            app.RecordSW.Text = '��ʼ�ɼ�';
            pause(0.1);
        end
        
    end
    

    methods (Access = private)

        % Button pushed function: RecordSW
        function RecordSWPushed(app, event)
            sComPortNum = app.UARTPort.Value;
            RecBaud = app.RecordBaud.Value;
            sComPort = serial(sComPortNum, 'BAUD', RecBaud);
            sComPortAct = instrfind({'PORT','Status'},{sComPort.port,'open'});
            if(~isempty(sComPortAct))
                fclose(sComPortAct);
            end
            RecTimes = app.RecordTimes.Value;
            RecLen = app.RecordLen.Value + 1;
            RecFrameSize = app.FrameSize.Value;
            RecFilenamePre = app.FilenamePrefix.Value;
            if(app.RecRuning ~= 1)
                RecStart(app,sComPort,RecTimes,RecLen,RecFrameSize,RecFilenamePre,RecBaud);
                switch(app.RecRuning)
                    case 1
                        app.RecordStatus.Color = 'r';
                        app.RecordSW.Text = 'ֹͣ�ɼ�';
                    case 0
                        app.RecordStatus.Color = 'g';
                        app.RecordSW.Text = '��ʼ�ɼ�';
                    case -1
                        app.RecordStatus.Color = 'y';
                        app.RecordSW.Text = '��������';
                    otherwise
                        app.RecordStatus.Color = 'g';
                        app.RecordSW.Text = '��ʼ�ɼ�';
                end
                pause(0.1);
            else
                RecStop(app,RecTimes,sComPort);
                switch(app.RecRuning)
                    case 1
                        app.RecordStatus.Color = 'r';
                        app.RecordSW.Text = 'ֹͣ�ɼ�';
                    case 0
                        app.RecordStatus.Color = 'g';
                        app.RecordSW.Text = '��ʼ�ɼ�';
                    case -1
                        app.RecordStatus.Color = 'y';
                        app.RecordSW.Text = '��������';
                    otherwise
                        app.RecordStatus.Color = 'g';
                        app.RecordSW.Text = '��ʼ�ɼ�';
                end
                pause(0.1);
            end
        end

        % Button pushed function: DispSW
        function DispSWButtonPushed(app, event)
            filename = ['.\' app.DataDirName '\' app.DispFilename.Value];
            Dispfile = fopen(filename, 'r');
            if(Dispfile <= 0)
                app.DispStatus.Color = 'y';
                return
            end
            pause(0.1);
            app.DispStatus.Color = 'r';
            app.DispSW.Text = '���ڶ�ȡ';
            pause(0.1);
            data = fread(Dispfile,'uint8');
            app.DispSW.Text = '���ڻ�ͼ';
            pause(0.1);
            frameheader = char([hex2dec('55') hex2dec('aa') hex2dec('aa') hex2dec('55') hex2dec('55') hex2dec('aa') hex2dec('aa') hex2dec('55')]);
            frameheadersize = length(frameheader);
            datachar = char(data);
            datastr = string(datachar');
            datas16 = [];
            index = strfind(datastr, frameheader);
            if(length(index)<2)
                app.DispStatus.Color = 'y';
                fclose(Dispfile);
                pause(0.1);
                return
            end
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
            DataLen = length(datas16);
            TimeLen = DataLen/8000;
            TimeMax = ceil(TimeLen);
            LenMax = TimeMax * 8000;
            TimeStep = floor(TimeMax/10);
            LenStep = TimeStep * 8000;
            XTickLoc = 0:LenStep:LenMax;
            xTickNum = 0:TimeStep:TimeMax;
            XTickStr = cellstr(num2str(xTickNum'))';
            plot(app.DispAxes,datas16);
            app.DispAxes.XLim = [0 LenMax];
            app.DispAxes.XTick = XTickLoc;
            app.DispAxes.XTickLabel = XTickStr;
            pause(1);
            app.DispSW.Text = '��ʾ����';
            app.DispStatus.Color = 'g';
            fclose(Dispfile);
            pause(0.1);
        end

        % Button pushed function: Bin2WavSW
        function Bin2WavSWButtonPushed(app, event)
            filedirstatus = mkdir(app.WavDirName);
            if(~filedirstatus)
                app.TransStatus.Color = 'y';
                return
            end
            filename = ['.\' app.DataDirName '\' app.DispFilename.Value];
            Binfile = fopen(filename, 'r');
            if(Binfile <= 0)
                app.TransStatus.Color = 'y';
                return
            end
            pause(0.1);
            app.TransStatus.Color = 'r';
            app.Bin2WavSW.Text = '���ڶ�ȡ';
            pause(0.1);
            data = fread(Binfile,'uint8');
            app.Bin2WavSW.Text = '����ת��';
            pause(0.1);
            frameheader = char([hex2dec('55') hex2dec('aa') hex2dec('aa') hex2dec('55') hex2dec('55') hex2dec('aa') hex2dec('aa') hex2dec('55')]);
            frameheadersize = length(frameheader);
            datachar = char(data);
            datastr = string(datachar');
            datas16 = [];
            index = strfind(datastr, frameheader);
            if(length(index)<2)
                app.TransStatus.Color = 'y';
                fclose(Binfile);
                pause(0.1);
                return
            end
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
            index = strfind(app.DispFilename.Value,'.bin');
            wavfilename = [app.DispFilename.Value(1:index) '.wav'];
            wavfile = ['.\' app.WavDirName '\' wavfilename];
            audiowrite(wavfile, datas16, 8000);
            pause(1);
            app.Bin2WavSW.Text = '���wav';
            app.TransStatus.Color = 'g';
            fclose(Binfile);
            pause(0.1);

        end
    end

    % App initialization and construction
    methods (Access = private)

        % Create UIFigure and components
        function createComponents(app)

            % Create ISD9160VoiceRecordUIFigure
            app.ISD9160VoiceRecordUIFigure = uifigure;
            app.ISD9160VoiceRecordUIFigure.Position = [100 100 640 480];
            app.ISD9160VoiceRecordUIFigure.Name = 'ISD9160 VoiceRecord';

            % Create TabGroup
            app.TabGroup = uitabgroup(app.ISD9160VoiceRecordUIFigure);
            app.TabGroup.Position = [1 1 583 480];

            % Create VoiceRecordTab
            app.VoiceRecordTab = uitab(app.TabGroup);
            app.VoiceRecordTab.Title = '¼��';

            % Create VoiceRecordPanel
            app.VoiceRecordPanel = uipanel(app.VoiceRecordTab);
            app.VoiceRecordPanel.Title = 'VoiceRecord';
            app.VoiceRecordPanel.Position = [1 1 581 452];

            % Create RecordSW
            app.RecordSW = uibutton(app.VoiceRecordPanel, 'push');
            app.RecordSW.ButtonPushedFcn = createCallbackFcn(app, @RecordSWPushed, true);
            app.RecordSW.Position = [51 273 100 24];
            app.RecordSW.Text = '��ʼ�ɼ�';

            % Create UARTEditFieldLabel
            app.UARTEditFieldLabel = uilabel(app.VoiceRecordPanel);
            app.UARTEditFieldLabel.HorizontalAlignment = 'right';
            app.UARTEditFieldLabel.Position = [32 367 62 22];
            app.UARTEditFieldLabel.Text = 'UART�˿�';

            % Create UARTPort
            app.UARTPort = uieditfield(app.VoiceRecordPanel, 'text');
            app.UARTPort.HorizontalAlignment = 'center';
            app.UARTPort.Position = [109 367 100 22];
            app.UARTPort.Value = 'COM3';

            % Create EditField_2Label
            app.EditField_2Label = uilabel(app.VoiceRecordPanel);
            app.EditField_2Label.HorizontalAlignment = 'right';
            app.EditField_2Label.Position = [295 329 77 22];
            app.EditField_2Label.Text = '���βɼ�����';

            % Create RecordLen
            app.RecordLen = uieditfield(app.VoiceRecordPanel, 'numeric');
            app.RecordLen.ValueDisplayFormat = '%.0f';
            app.RecordLen.HorizontalAlignment = 'center';
            app.RecordLen.Position = [387 329 100 22];
            app.RecordLen.Value = 3600;

            % Create EditField_3Label
            app.EditField_3Label = uilabel(app.VoiceRecordPanel);
            app.EditField_3Label.HorizontalAlignment = 'right';
            app.EditField_3Label.Visible = 'off';
            app.EditField_3Label.Position = [319 286 53 22];
            app.EditField_3Label.Text = '��Ƭ����';

            % Create FrameSize
            app.FrameSize = uieditfield(app.VoiceRecordPanel, 'numeric');
            app.FrameSize.ValueDisplayFormat = '%.0f';
            app.FrameSize.HorizontalAlignment = 'center';
            app.FrameSize.Visible = 'off';
            app.FrameSize.Position = [387 286 100 22];
            app.FrameSize.Value = 1;

            % Create EditField_4Label
            app.EditField_4Label = uilabel(app.VoiceRecordPanel);
            app.EditField_4Label.HorizontalAlignment = 'right';
            app.EditField_4Label.Position = [307 241 65 22];
            app.EditField_4Label.Text = '�ļ���ǰ׺';

            % Create FilenamePrefix
            app.FilenamePrefix = uieditfield(app.VoiceRecordPanel, 'text');
            app.FilenamePrefix.HorizontalAlignment = 'center';
            app.FilenamePrefix.Position = [387 241 100 22];
            app.FilenamePrefix.Value = 'recordtest';

            % Create EditFieldLabel
            app.EditFieldLabel = uilabel(app.VoiceRecordPanel);
            app.EditFieldLabel.HorizontalAlignment = 'right';
            app.EditFieldLabel.Position = [319 374 53 22];
            app.EditFieldLabel.Text = '�ɼ�����';

            % Create RecordTimes
            app.RecordTimes = uieditfield(app.VoiceRecordPanel, 'numeric');
            app.RecordTimes.ValueDisplayFormat = '%.0f';
            app.RecordTimes.HorizontalAlignment = 'center';
            app.RecordTimes.Position = [387 374 100 22];
            app.RecordTimes.Value = 3;

            % Create RecordStatus
            app.RecordStatus = uilamp(app.VoiceRecordPanel);
            app.RecordStatus.Position = [171 275 20 20];

            % Create Label
            app.Label = uilabel(app.VoiceRecordPanel);
            app.Label.HorizontalAlignment = 'right';
            app.Label.Position = [53 329 41 22];
            app.Label.Text = '������';

            % Create RecordBaud
            app.RecordBaud = uieditfield(app.VoiceRecordPanel, 'numeric');
            app.RecordBaud.ValueDisplayFormat = '%.0f';
            app.RecordBaud.HorizontalAlignment = 'center';
            app.RecordBaud.Position = [109 329 100 22];
            app.RecordBaud.Value = 921600;

            % Create Label_2
            app.Label_2 = uilabel(app.VoiceRecordPanel);
            app.Label_2.HorizontalAlignment = 'right';
            app.Label_2.Position = [42 190 53 22];
            app.Label_2.Text = '��������';

            % Create DispWindow
            app.DispWindow = uitextarea(app.VoiceRecordPanel);
            app.DispWindow.Position = [110 43 384 171];

            % Create DispData
            app.DispData = uicheckbox(app.VoiceRecordPanel);
            app.DispData.Text = '��ʾ��������';
            app.DispData.Position = [54 241 94 22];

            % Create FileDisplayTab
            app.FileDisplayTab = uitab(app.TabGroup);
            app.FileDisplayTab.Title = '��ȡ';

            % Create FileDisplayPanel
            app.FileDisplayPanel = uipanel(app.FileDisplayTab);
            app.FileDisplayPanel.Title = 'FileDisplay';
            app.FileDisplayPanel.Position = [1 1 581 452];

            % Create DispSW
            app.DispSW = uibutton(app.FileDisplayPanel, 'push');
            app.DispSW.ButtonPushedFcn = createCallbackFcn(app, @DispSWButtonPushed, true);
            app.DispSW.Position = [386 386 100 24];
            app.DispSW.Text = '��ʾ����';

            % Create Label_3
            app.Label_3 = uilabel(app.FileDisplayPanel);
            app.Label_3.HorizontalAlignment = 'right';
            app.Label_3.Position = [34 387 41 22];
            app.Label_3.Text = '�ļ���';

            % Create DispFilename
            app.DispFilename = uieditfield(app.FileDisplayPanel, 'text');
            app.DispFilename.HorizontalAlignment = 'center';
            app.DispFilename.Position = [90 387 272 22];
            app.DispFilename.Value = 'recordtest.bin';

            % Create DispStatus
            app.DispStatus = uilamp(app.FileDisplayPanel);
            app.DispStatus.Position = [503 388 20 20];

            % Create DispAxes
            app.DispAxes = uiaxes(app.FileDisplayPanel);
            title(app.DispAxes, 'Record ADC Data')
            xlabel(app.DispAxes, 'Time (s)')
            ylabel(app.DispAxes, 'ADC Data')
            app.DispAxes.YLim = [-40000 40000];
            app.DispAxes.Box = 'on';
            app.DispAxes.XTick = [0 0.1 0.2 0.3 0.4 0.5 0.6 0.7 0.8 0.9 1];
            app.DispAxes.YTickLabel = {'-40000'; '-30000'; '-20000'; '-10000'; '0'; '10000'; '20000'; '30000'; '40000'};
            app.DispAxes.Position = [34 22 513 314];

            % Create Bin2WavSW
            app.Bin2WavSW = uibutton(app.FileDisplayPanel, 'push');
            app.Bin2WavSW.ButtonPushedFcn = createCallbackFcn(app, @Bin2WavSWButtonPushed, true);
            app.Bin2WavSW.Position = [386 353 100 24];
            app.Bin2WavSW.Text = '���wav';

            % Create TransStatus
            app.TransStatus = uilamp(app.FileDisplayPanel);
            app.TransStatus.Position = [503 355 20 20];
        end
    end

    methods (Access = public)

        % Construct app
        function app = VoiceRecordnl_exported

            % Create and configure components
            createComponents(app)

            % Register the app with App Designer
            registerApp(app, app.ISD9160VoiceRecordUIFigure)

            if nargout == 0
                clear app
            end
        end

        % Code that executes before app deletion
        function delete(app)

            % Delete UIFigure when app is deleted
            delete(app.ISD9160VoiceRecordUIFigure)
        end
    end
end