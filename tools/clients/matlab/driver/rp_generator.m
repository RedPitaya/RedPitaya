function varargout = rp_generator(varargin)
% RP_GENERATOR MATLAB code for rp_generator.fig
%      RP_GENERATOR, by itself, creates a new RP_GENERATOR or raises the existing
%      singleton*.
%
%      H = RP_GENERATOR returns the handle to a new RP_GENERATOR or the handle to
%      the existing singleton*.
%
%      RP_GENERATOR('CALLBACK',hObject,eventData,handles,...) calls the local
%      function named CALLBACK in RP_GENERATOR.M with the given input arguments.
%
%      RP_GENERATOR('Property','Value',...) creates a new RP_GENERATOR or raises
%      the existing singleton*.  Starting from the left, property value pairs are
%      applied to the GUI before rp_generator_OpeningFcn gets called.  An
%      unrecognized property name or invalid value makes property application
%      stop.  All inputs are passed to rp_generator_OpeningFcn via varargin.
%
%      *See GUI Options on GUIDE's Tools menu.  Choose "GUI allows only one
%      instance to run (singleton)".
%
% See also: GUIDE, GUIDATA, GUIHANDLES

% Edit the above text to modify the response to help rp_generator

% Last Modified by GUIDE v2.5 02-Mar-2015 14:21:41

% Begin initialization code - DO NOT EDIT
gui_Singleton = 1;
gui_State = struct('gui_Name',       mfilename, ...
    'gui_Singleton',  gui_Singleton, ...
    'gui_OpeningFcn', @rp_generator_OpeningFcn, ...
    'gui_OutputFcn',  @rp_generator_OutputFcn, ...
    'gui_LayoutFcn',  [] , ...
    'gui_Callback',   []);
if nargin && ischar(varargin{1})
    gui_State.gui_Callback = str2func(varargin{1});
end

if nargout
    [varargout{1:nargout}] = gui_mainfcn(gui_State, varargin{:});
else
    gui_mainfcn(gui_State, varargin{:});
end
% End initialization code - DO NOT EDIT

% --- Executes just before rp_generator is made visible.
function rp_generator_OpeningFcn(hObject, eventdata, handles, varargin)
% This function has no output args, see OutputFcn.
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
% varargin   command line arguments to rp_generator (see VARARGIN)

% if (strcmp(interfaceObj.Terminator(1), 'CR/LF'));
%     error('Terminator is not set correctly!');
% end
% Create a device object.

% Choose default command line output for rp_generator

%interfaceObj = tcpip('192.168.178.66', 5000);
%deviceObj = icdevice('red_pitaya.mdd', interfaceObj);
handles.device = 0;
%delete(deviceObj);



handles.output = hObject;
% Update handles structure
guidata(hObject, handles);

initialize_gui(hObject, handles, false);

% UIWAIT makes rp_generator wait for user response (see UIRESUME)
% uiwait(handles.figure1);


% --- Outputs from this function are returned to the command line.
function varargout = rp_generator_OutputFcn(hObject, eventdata, handles)
% varargout  cell array for returning output args (see VARARGOUT);
% hObject    handle to figure
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Get default command line output from handles structure
varargout{1} = handles.output;


% --- Executes during object creation, after setting all properties.
function density_CreateFcn(hObject, eventdata, handles)
% hObject    handle to density (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end




% --- Executes on button press in reset.
function reset_Callback(hObject, eventdata, handles)
% hObject    handle to reset (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

%reset all values
%ch1
handles.asgdata.enablech1 = 'OFF';
handles.asgdata.frequencych1 = 1000;
handles.asgdata.fr_prefixch1 = 1e0;
handles.asgdata.typech1 = 'SINE';
handles.asgdata.amplitudech1 = 1;
handles.asgdata.phasech1 = 0;
handles.asgdata.offsetch1 = 0;
ndles.asgdata.burst_signal_periodesch1= 1;
handles.asgdata.dutycyclech1 = 50;
handles.asgdata.num_burstsch1 = 1;
handles.asgdata.burst_periodch1 = 1000000;
handles.asgdata.EnableBurstch1 = 0;
handles.asgdata.triggersourcech1 = 'INT';
handle.asgdata.arbitrarydatach1 = zeros(1,16000);
handle.asgdata.ImmediateTgiggerch1 = 0;

%ch2
handles.asgdata.enablech1 = 'OFF';
handles.asgdata.frequencych2 = 1000;
handles.asgdata.fr_prefixch2 = 1e0;
handles.asgdata.typech2 = 'SINE';
handles.asgdata.amplitudech2 = 1;
handles.asgdata.phasech2 = 0;
handles.asgdata.offsetch2 = 0;
handles.asgdata.dutycyclech2 = 50;
handles.asgdata.burst_signal_cyclesch2 = 1;
handles.asgdata.num_burstsch2 = 1;
handles.asgdata.burst_periodch2 = 1000000;
handles.asgdata.EnableBurstch2 = 0;
handles.asgdata.triggersourcech2 = 'INT';
handle.asgdata.arbitrarydatach2 = zeros(1,16000);
handle.asgdata.ImmediateTgiggerch2 = 0;

parents=[];
parents(1) = get(hObject,'parent');
interfaceelements = get(parents(1),'Children');
children = get(parents(1),'Children');
deviceObj = handles.device;


%find all panels
panel_ids = [];
counter_panel_id = 2;

%load all panel ids
for childid = 1 :  length(children)
    %get(children(childid),'Children')
    %childid
    if (length(findstr(get(children(childid), 'Tag'),'panel'))>0)
        parents(counter_panel_id) =  childid;
        counter_panel_id = counter_panel_id + 1;
    end
end

% panel_children = get(children(16),'Children')
% get(panel_children(1))

for id = 1 : length(parents)
    
    if (id > 1) %use loaded child ids and load subitems to 'children'
        children = get(interfaceelements(parents(id)),'Children');
    else % load main elements form the interface to 'children'
        children = get(parents(id),'Children');
    end
    
    for childid = 1 :length(children) % search through the elements and set its values
        
        if strcmp(get(children(childid), 'Tag'),'enable_ch1')
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            set(children(childid),'BackgroundColor', [0.9412    0.9412    0.9412]);
            invoke(groupObj, 'enable', 'OFF');
            
        elseif strcmp(get(children(childid), 'Tag'),'frequency_input_ch1')
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            invoke(groupObj, 'frequency', '1000');
            set(children(childid),'String', 1000);
            
        elseif strcmp(get(children(childid), 'Tag'),'Fr_prefix_ch1')
            set(children(childid),'Value', 1e0);
            
        elseif strcmp(get(children(childid), 'Tag'),'amplitude_input_ch1')
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            invoke(groupObj, 'Amplitude', '1');
            set(children(childid),'String', '1');
            
        elseif strcmp(get(children(childid), 'Tag'),'phase_input_ch1')
            
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            invoke(groupObj, 'Phase', '0');
            set(children(childid),'String', '0');
            
            
        elseif strcmp(get(children(childid), 'Tag'),'duty_cycle_input_ch1')
            
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            invoke(groupObj, 'DutyCycle', '50');
            set(children(childid),'String', '50');
            
        elseif strcmp(get(children(childid), 'Tag'),'offset_input_ch1')
            
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            invoke(groupObj, 'Offset', '0');
            set(children(childid),'String', '0');
            
            
        elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            invoke(groupObj, 'Waveform', 'SINE');
            set(children(childid),'BackgroundColor', [0.68  0.92 1]);
        elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch1')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
            
            
        elseif strcmp(get(children(childid), 'Tag'),'enable_burst_togg_ch1')
            
            groupObj = get(deviceObj, 'Burstmodch1');
            
            invoke(groupObj, 'Enabled', 'OFF');
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
            
            for childidrst = 1 :  length(children)
                if strcmp(get(children(childidrst), 'Tag'),'trigg_imm_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'number_ofcycles_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'num_bursts_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'burst_period_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigg_source_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_N_radio_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_P_radio_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_internal_radio_ch1')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_gated_radio_ch1')
                    set(children(childidrst),'Enable','off');
                end
                
            end
            
            
        elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
            
            groupObj = get(deviceObj, 'Triggerch1');
            
            invoke(groupObj, 'Source', 'INT');
            
            for childidrst = 1 :  length(children)
                if strcmp(get(children(childidrst), 'Tag'),'trigger_gated_radio_ch1')
                    set(children(childidrst),'Value',0);
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_P_radio_ch1')
                    set(children(childidrst),'Value',0);
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_N_radio_ch1')
                    set(children(childidrst),'Value',0);
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_internal_radio_ch1')
                    set(children(childidrst),'Value',1);
                end
            end
            
            
        elseif strcmp(get(children(childid), 'Tag'),'number_ofcycles_ch1')
            set(children(childid),'String', '1');
            groupObj = get(deviceObj, 'Burstmodch1');
            
            invoke(groupObj, 'Cycles', '1');
        elseif strcmp(get(children(childid), 'Tag'),'num_bursts_ch1')
            set(children(childid),'String', '1');
            groupObj = get(deviceObj, 'Burstmodch1');
            
            invoke(groupObj, 'BurstCount', '1');
            
        elseif strcmp(get(children(childid), 'Tag'),'burst_period_ch1')
            set(children(childid),'String', '1000000');
            groupObj = get(deviceObj, 'Burstmodch1');
            
            invoke(groupObj, 'InternalPeriod', '1000000');
            
        end
        
        
        
        if strcmp(get(children(childid), 'Tag'),'enable_ch2')
            groupObj = get(deviceObj, 'Arbitrarywaveformch2');
            
            set(children(childid),'BackgroundColor', [0.9412    0.9412    0.9412]);
            invoke(groupObj, 'enable', 'OFF');
            
        elseif strcmp(get(children(childid), 'Tag'),'frequency_input_ch2')
            groupObj = get(deviceObj, 'Arbitrarywaveformch2');
            
            invoke(groupObj, 'frequency', '1000');
            set(children(childid),'String', 1000);
            
        elseif strcmp(get(children(childid), 'Tag'),'Fr_prefix_ch2')
            set(children(childid),'Value', 1e0);
            
        elseif strcmp(get(children(childid), 'Tag'),'amplitude_input_ch2')
            groupObj = get(deviceObj, 'Arbitrarywaveformch2');
            
            invoke(groupObj, 'Amplitude', '1');
            set(children(childid),'String', '1');
            
        elseif strcmp(get(children(childid), 'Tag'),'phase_input_ch2')
            
            groupObj = get(deviceObj, 'Arbitrarywaveformch2');
            
            invoke(groupObj, 'Phase', '0');
            set(children(childid),'String', '0');
            
            
        elseif strcmp(get(children(childid), 'Tag'),'duty_cycle_input_ch2')
            
            groupObj = get(deviceObj, 'Arbitrarywaveformch2');
            
            invoke(groupObj, 'DutyCycle', '50');
            set(children(childid),'String', '50');
            
        elseif strcmp(get(children(childid), 'Tag'),'offset_input_ch2')
            groupObj = get(deviceObj, 'Arbitrarywaveformch2');
            
            invoke(groupObj, 'Offset', '0');
            set(children(childid),'String', '0');
            
            
        elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
            groupObj = get(deviceObj, 'Arbitrarywaveformch1');
            
            invoke(groupObj, 'Waveform', 'SINE');
            set(children(childid),'BackgroundColor', [0.68  0.92 1]);
        elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch2')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
        elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
            
            
        elseif strcmp(get(children(childid), 'Tag'),'enable_burst_togg_ch2')
            groupObj = get(deviceObj, 'Burstmodch2');
            
            invoke(groupObj, 'Enabled', 'OFF');
            set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
            
            for childidrst = 1 :  length(children)
                if strcmp(get(children(childidrst), 'Tag'),'trigg_imm_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'number_ofcycles_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'num_bursts_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'burst_period_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigg_source_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_internal_radio_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_N_radio_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_P_radio_ch2')
                    set(children(childidrst),'Enable','off');
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_gated_radio_ch2')
                    set(children(childidrst),'Enable','off');
                end
                
            end
            
            
        elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
            
            groupObj = get(deviceObj, 'Triggerch2');
            
            invoke(groupObj, 'Source', 'INT');
            
            for childidrst = 1 :  length(children)
                if strcmp(get(children(childidrst), 'Tag'),'trigger_gated_radio_ch2')
                    set(children(childidrst),'Value',0);
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_N_radio_ch2')
                    set(children(childidrst),'Value',0);
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_external_P_radio_ch2')
                    set(children(childidrst),'Value',0);
                elseif strcmp(get(children(childidrst), 'Tag'),'trigger_internal_radio_ch2')
                    set(children(childidrst),'Value',1);
                end
            end
            
            
        elseif strcmp(get(children(childid), 'Tag'),'number_ofcycles_ch2')
            set(children(childid),'String', '1');
            groupObj = get(deviceObj, 'Burstmodch2');
            
            invoke(groupObj, 'Cycles', '1');
        elseif strcmp(get(children(childid), 'Tag'),'num_bursts_ch2')
            set(children(childid),'String', '1');
            groupObj = get(deviceObj, 'Burstmodch2');
            
            invoke(groupObj, 'BurstCount', '1');
            
        elseif strcmp(get(children(childid), 'Tag'),'burst_period_ch2')
            set(children(childid),'String', '1000000');
            groupObj = get(deviceObj, 'Burstmodch2');
            
            invoke(groupObj, 'InternalPeriod', '1000000');
            
        end
        
    end %search through for selected tags
end
end %if connection is active 


% --- Executes when selected object changed in unitgroup.
function unitgroup_SelectionChangeFcn(hObject, eventdata, handles)
% hObject    handle to the selected object in unitgroup
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (hObject == handles.english)
    set(handles.text4, 'String', 'lb/cu.in');
    set(handles.text5, 'String', 'cu.in');
    set(handles.text6, 'String', 'lb');
else
    set(handles.text4, 'String', 'kg/cu.m');
    set(handles.text5, 'String', 'cu.m');
    set(handles.text6, 'String', 'kg');
end

% --------------------------------------------------------------------
function initialize_gui(fig_handle, handles, isreset)
% If the metricdata field is present and the reset flag is false, it means
% we are just re-initializing a GUI by calling it from the cmd line
% while it is up. So, bail out as we dont want to reset the data.
if isfield(handles, 'metricdata') && ~isreset
    return;
end


%Red Pitaya

handles.asgdata.frequencyMAX = 62.5e6;
handles.asgdata.frequencyMIN = 0;
handles.IP = '192.168.178.66';
handles.port = 5000;
handles.asgdata.connectionstate = 'offline';
handles.asgdata.connectmessage = 'Please connect to the instrument first. If you are experiencing difficulties search for instructions or support at www.redpitaya.com';
handles.asgdata.num_bursts_MAX = 50000;

%ch1
handles.asgdata.enablech1 = 'OFF';
handles.asgdata.frequencych1 = 1000;
handles.asgdata.fr_prefixch1 = 1e0;
handles.asgdata.typech1 = 'SINE';
handles.asgdata.amplitudech1 = 1;
handles.asgdata.phasech1 = 0;
handles.asgdata.offsetch1 = 0;
handles.asgdata.dutycyclech1 = 50;
handles.asgdata.burst_signal_periodesch1= 1;
handles.asgdata.num_burstsch1 = 1;
handles.asgdata.burst_periodch1 = 10000;
handles.asgdata.EnableBurstch1 = 0;
handles.asgdata.triggersourcech1 = 'INT';
handle.asgdata.arbitrarydatach1 = zeros(1,16000);
handle.asgdata.ImmediateTgiggerch1 = 0;

%ch2
handles.asgdata.enablech1 = 'OFF';
handles.asgdata.frequencych2 = 1000;
handles.asgdata.fr_prefixch2 = 1e0;
handles.asgdata.typech2 = 'SINE';
handles.asgdata.amplitudech2 = 1;
handles.asgdata.phasech2 = 0;
handles.asgdata.offsetch2 = 0;
handles.asgdata.dutycyclech2 = 50;
handles.asgdata.burst_signal_periodesch2= 1;
handles.asgdata.num_burstsch2 = 1;
handles.asgdata.burst_periodch2 = 1000000;
handles.asgdata.EnableBurstch2 = 0;
handles.asgdata.triggersourcech2 = 'INT';
handle.asgdata.arbitrarydatach2 = zeros(1,16000);
handle.asgdata.ImmediateTgiggerch2 = 0;




% Update handles structure
guidata(handles.figure1, handles);


% --- Executes on selection change in Fr_prefix_ch1.
function Fr_prefix_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to Fr_prefix_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns Fr_prefix_ch1 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from Fr_prefix_ch1

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

val = get(hObject, 'Value');
str = get(hObject, 'String');
switch str{val}
    case 'Hz' % User selects Hz
        handles.asgdata.fr_prefixch1  = 1e0;
    case 'KHz' % User selects kHz
        handles.asgdata.fr_prefixch1  = 1e3;
    case 'MHz' % User selects MHz
        handles.asgdata.fr_prefixch1  = 1e6;
end


frequency = handles.asgdata.frequencych1 * handles.asgdata.fr_prefixch1;

if (frequency > handles.asgdata.frequencyMAX)
    errordlg('Frequnecy excedes maximum value!');
    
elseif(frequency < handles.asgdata.frequencyMIN)
    errordlg('Frequnecy excedes minimum value!');
end
    
    frequencystr = num2str(frequency);
    deviceObj = handles.device;
    groupObj = get(deviceObj, 'Arbitrarywaveformch1');
    
    invoke(groupObj, 'frequency', frequencystr);
    
    
    guidata(hObject,handles);

end

% --- Executes during object creation, after setting all properties.
function Fr_prefix_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Fr_prefix_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function frequency_input_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to frequency_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of frequency_input_ch1 as text
%        str2double(get(hObject,'String')) returns contents of frequency_input_ch1 as a double

if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else
    
frequencyinput = (str2double(get(hObject, 'String')));

if isnan(frequencyinput)
    set(hObject, 'String', 1000);
    errordlg('Input must be a number','Error');
end
frequency = frequencyinput * handles.asgdata.fr_prefixch1;
if (frequency > handles.asgdata.frequencyMAX)
    errordlg('Frequnecy excedes maximum value!');
    
elseif(frequency < handles.asgdata.frequencyMIN)
    errordlg('Frequnecy excedes minimum value!');
end

frequencystr = num2str(frequency);

deviceObj = handles.device;

groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'frequency', frequencystr);

handles.asgdata.frequencych1 = frequencyinput;
guidata(hObject,handles);
end

% --- Executes during object creation, after setting all properties.
function frequency_input_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to frequency_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function offset_input_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to offset_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of offset_input_ch1 as text
%        str2double(get(hObject,'String')) returns contents of offset_input_ch1 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

offset = str2double(get(hObject, 'String'));
if isnan(offset)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if ((abs(handles.asgdata.amplitudech1)+abs(offset)) > 1)
    errordlg('Output amplitude can not be greater than 1V!');
end
offset = (str2double(get(hObject, 'String')));

if isnan(offset)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if (offset >= 1)
    errordlg('Offset excedes maximum value!');
    
elseif(offset <= -1)
    errordlg('Offset excedes minimum value!');
end

offsetstr = num2str(offset);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'Offset', offsetstr);

handles.asgdata.offsetch1 = offset;
guidata(hObject,handles);
end

% --- Executes during object creation, after setting all properties.
function offset_input_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to offset_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function amplitude_input_ch12_Callback(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch11 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of amplitude_input_ch11 as text
%        str2double(get(hObject,'String')) returns contents of amplitude_input_ch11 as a double
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

amplitude = (str2double(get(hObject, 'String')));

if isnan(amplitude)
    set(hObject, 'String', 1);
    errordlg('Input must be a number','Error');
end

if ((abs(handles.asgdata.offsetch1)+abs(amplitude)) > 1)
    errordlg('Output amplitude can not be greater than 1V!');
end

if (amplitude > 1)
    errordlg('Amplitude excedes maximum value!');
    
elseif(amplitude < 0)
    errordlg('Amplitude excedes minimum value!');
end

amplitudestr = num2str(amplitude);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'Amplitude', amplitudestr);

handles.asgdata.amplitude1 = amplitude;
guidata(hObject,handles);
end

% --- Executes during object creation, after setting all properties.
function amplitude_input_ch11_CreateFcn(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch11 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function duty_cycle_input_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to duty_cycle_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of duty_cycle_input_ch1 as text
%        str2double(get(hObject,'String')) returns contents of duty_cycle_input_ch1 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

dutycycle = (str2double(get(hObject, 'String')));

if isnan(dutycycle)
    set(hObject, 'String', 1);
    errordlg('Input must be a number','Error');
end

if (dutycycle > 100)
    errordlg('Duty Cycle excedes maximum value!');
    
elseif(dutycycle < 0)
    errordlg('Duty Cycle excedes minimum value!');
end

dutycyclestr = num2str(dutycycle);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'DutyCycle', dutycyclestr);

handles.asgdata.dutycyclech1 = dutycycle;
guidata(hObject,handles);

end

% --- Executes during object creation, after setting all properties.
function duty_cycle_input_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to duty_cycle_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function phase_input_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to phase_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of phase_input_ch1 as text
%        str2double(get(hObject,'String')) returns contents of phase_input_ch1 as a double

if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

phase = (str2double(get(hObject, 'String')));

if isnan(phase)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if (phase > 360)
    errordlg('Value must not be greater than 360!');
    
elseif(phase < -360)
    errordlg('Phase must not be smaller than -360!');
end

phasestr = num2str(phase);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'Phase', phasestr);

handles.asgdata.phasech1 = phase;
guidata(hObject,handles);
end
% --- Executes during object creation, after setting all properties.
function phase_input_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to phase_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in SINE_togg_ch1.
function SINE_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to SINE_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of SINE_togg_ch1

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;

groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'waveform', 'SINE');

parent = get(hObject,'parent');
children = get(parent,'Children');


for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech1 = 'SINE';
guidata(hObject,handles);
end
% --- Executes on button press in togglebutton2.
function togglebutton2_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton2


% --- Executes on button press in togglebutton3.
function togglebutton3_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton3 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton3


% --- Executes on button press in togglebutton4.
function togglebutton4_Callback(hObject, eventdata, handles)
% hObject    handle to togglebutton4 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of togglebutton4


% --- Executes on button press in TRI_togg_ch1.
function TRI_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to TRI_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of TRI_togg_ch1

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'waveform', 'TRIANGLE');

parent = get(hObject,'parent');
children = get(parent,'Children');

for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech1 = 'TRIANGLE';
guidata(hObject,handles);
end
% --- Executes on button press in SQR_togg_ch1.
function SQR_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to SQR_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of SQR_togg_ch1

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'waveform', 'SQUARE');


parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech1 = 'SQR';
guidata(hObject,handles);
end
% --- Executes on button press in SAWU_togg_ch1.
function SAWU_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to SAWU_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of SAWU_togg_ch1

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'waveform', 'SAWU');

parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech1 = 'SAWU';
guidata(hObject,handles);
end
% --- Executes on button press in SAWD_togg_ch1.
function SAWD_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to SAWD_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'waveform', 'SAWD');

parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end
handles.asgdata.typech1 = 'SAWD';
guidata(hObject,handles);
end
% --- Executes on button press in PWM_togg_ch1.
function PWM_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to PWM_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');
 
invoke(groupObj, 'waveform', 'PWM');


parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'PWM_togg_ch1')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end
handles.asgdata.typech1 = 'PWM';
guidata(hObject,handles);
end
% --- Executes on button press in ARB_togg_ch1.
function ARB_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to ARB_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of ARB_togg_ch1

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');
 
invoke(groupObj, 'waveform', 'ARBITRARY');


parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'ARB_togg_ch1')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch1')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech1 = 'ARB';
guidata(hObject,handles);
end
function arbitrary_file_insert_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to arbitrary_file_insert_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of arbitrary_file_insert_ch1 as text
%        str2double(get(hObject,'String')) returns contents of arbitrary_file_insert_ch1 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
end


% --- Executes during object creation, after setting all properties.
function arbitrary_file_insert_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to arbitrary_file_insert_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in enable_burst_togg_ch1.
function enable_burst_togg_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to enable_burst_togg_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of enable_burst_togg_ch1

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;

groupObj = get(deviceObj, 'Triggerch1');
burstObj = get(deviceObj, 'Burstmodch1');

parent = get(hObject,'parent');
children = get(parent,'Children');
%get(children(1))


groupObj = get(deviceObj, 'Triggerch1');

handles.asgdata.triggersourcech1 = 'INT';

if((get(hObject,'BackgroundColor') == [0.68  0.92 1]))
    set(hObject,'BackgroundColor',[0.9412    0.9412    0.9412]);
    invoke(burstObj, 'Enabled', 'OFF');
    invoke(groupObj, 'Source', 'INT');
    handles.asgdata.EnableBurstch1 = 0;
    for childid = 1 :  length(children)
        if strcmp(get(children(childid), 'Tag'),'trigg_imm_ch1')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'number_ofcycles_ch1')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'num_bursts_ch1')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'burst_period_ch1')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'trigg_source_ch1')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch1')
            set(children(childid),'Value',0);
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch1')
            set(children(childid),'Value',0);
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
            set(children(childid),'Value',1);
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch1')
            set(children(childid),'Enable','off');
        end
   
    end
    
    %set fr again
    frequency = handles.asgdata.frequencych1 * handles.asgdata.fr_prefixch1;
    frequencystr = num2str(frequency);
    groupObj = get(deviceObj, 'Arbitrarywaveformch1');
    invoke(groupObj, 'frequency', frequencystr);
    
else
    set(hObject,'BackgroundColor',[0.68  0.92 1]);
    invoke(burstObj, 'Enabled', 'ON');
    invoke(groupObj, 'Source', 'INT');
    handles.asgdata.EnableBurstch1 = 1;
    for childid = 1 :  length(children)
        if strcmp(get(children(childid), 'Tag'),'trigg_imm_ch1')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'number_ofcycles_ch1')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'num_bursts_ch1')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'burst_period_ch1')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'trigg_source_ch1')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch1')
            set(children(childid),'Value',0);
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch1')
            set(children(childid),'Value',0);
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
            set(children(childid),'Value',1);
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch1')
            set(children(childid),'Value',0);
            set(children(childid),'Enable','on');
        end
        
    end
    
    %send default values
    groupObj = get(deviceObj, 'Burstmodch1');
    invoke(groupObj, 'BurstCount', num2str(handles.asgdata.num_burstsch1));
    invoke(groupObj, 'Cycles', num2str(handles.asgdata.burst_signal_periodesch1));
    invoke(groupObj, 'InternalPeriod', num2str(handles.asgdata.burst_periodch1));

    parent = get(hObject,'parent');
    child = findobj(parent,'Tag','number_ofcycles_ch1');%find object
    set(child,'String',num2str(handles.asgdata.burst_signal_periodesch1)); %Set string to graphical element
    
    child = findobj(parent,'Tag','num_bursts_ch1');%find object
    set(child,'String',num2str(handles.asgdata.num_burstsch1)); %Set string to graphical element
    
    child = findobj(parent,'Tag','burst_period_ch1');%find object
    set(child,'String',num2str(handles.asgdata.burst_periodch1)); %Set string to graphical element
end

guidata(hObject,handles);
end
function number_ofcycles_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to number_ofcycles_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of number_ofcycles_ch1 as text
%        str2double(get(hObject,'String')) returns contents of number_ofcycles_ch1 as a double
N_of_cycles = get(hObject,'String');
if(length(N_of_cycles) < 1)
    errordlg('Missing parameter!');
end
if (N_of_cycles < 1)
    error('Error value must be greater than 0!');
end

deviceObj = handles.device;
groupObj = get(deviceObj, 'Burstmodch1');
 
invoke(groupObj, 'Cycles', num2str(N_of_cycles));
invoke(groupObj, 'Enabled', 'ON');

handles.asgdata.burst_signal_periodesch1 = N_of_cycles;
guidata(hObject,handles);



% --- Executes during object creation, after setting all properties.
function number_ofcycles_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to number_ofcycles_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function num_bursts_Callback(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of num_bursts_ch2 as text
%        str2double(get(hObject,'String')) returns contents of num_bursts_ch2 as a double
N_of_bursts = get(hObject,'String');
if (N_of_bursts < 1)
    error('Error value must be greater than 0!');
end
deviceObj = handles.device;
groupObj = get(deviceObj, 'Burstmodch2');
 
N_of_bursts_str = num2str(N_of_bursts);
invoke(groupObj, 'BurstCount', N_of_bursts_str);
handles.asgdata.num_burstsch2 = N_of_bursts_str;
guidata(hObject,handles);





% --- Executes during object creation, after setting all properties.
function num_bursts_CreateFcn(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function burst_period_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to burst_period_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of burst_period_ch1 as text
%        str2double(get(hObject,'String')) returns contents of burst_period_ch1 as a double
period = get(hObject,'String');
if(length(period) < 1)
    errordlg('Missing parameter!');
end
if (period < 1)
    error('Error value must be greater than 0!');
end
deviceObj = handles.device;
groupObj = get(deviceObj, 'Burstmodch1');

invoke(groupObj, 'InternalPeriod', period);
invoke(groupObj, 'Enabled', 'ON');

handles.asgdata.burst_periodch1 = period;
guidata(hObject,handles);

% --- Executes during object creation, after setting all properties.
function burst_period_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to burst_period_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



% --- Executes on button press in reset.
function reset_g1_Callback(hObject, eventdata, handles)
% hObject    handle to reset (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

deviceObj = handles.device;
groupObj = get(deviceObj, 'System');

invoke(groupObj, 'ResetGenerator');

parent = get(hObject,'parent');
children = get(parent,'Children');

for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'trigg_imm_ch2')
        set(children(childid),'Value',1000);
    elseif strcmp(get(children(childid), 'Tag'),'number_ofcycles_ch2')
        set(children(childid),'Enable','off');
    elseif strcmp(get(children(childid), 'Tag'),'num_bursts_ch2')
        set(children(childid),'Enable','off');
    elseif strcmp(get(children(childid), 'Tag'),'burst_period_ch2')
        set(children(childid),'Enable','off');
    elseif strcmp(get(children(childid), 'Tag'),'trigg_source_ch2')
        set(children(childid),'Enable','off');
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_radio_ch2')
        set(children(childid),'Enable','off');
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
        set(children(childid),'Enable','off');
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
        set(children(childid),'Enable','off');
    end
    
end




% --- Executes on button press in pushbutton11.
function pushbutton11_Callback(hObject, eventdata, handles)
% hObject    handle to pushbutton11 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in enable_ch1.
function enable_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to enable_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else
deviceObj = handles.device;
% Execute device object function(s).
groupObj = get(deviceObj, 'Arbitrarywaveformch1');


if((get(hObject,'BackgroundColor') == [0,1,0]))
    set(hObject,'BackgroundColor',[0.9412    0.9412    0.9412]);
    invoke(groupObj, 'enable', 'OFF');
    handle.asgdata.enablech1 = 'OFF';
else
    set(hObject,'BackgroundColor','green');
    invoke(groupObj, 'enable', 'ON');
    handle.asgdata.enablech1 = 'ON';
end

guidata(hObject,handles);
end
% --- Executes on button press in load_btn_ch1.
function load_btn_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to load_btn_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

parent = get(hObject,'parent');
children = get(parent,'Children');
for childtag = 1:length(children)
    if strcmp(get(children(childtag), 'Tag'),'arbitrary_file_insert_ch1')
        file =  get(children(childtag),'String');
    end
end
data=[];

if (exist(file)==2)
    data= dlmread(file);
    %data = csvread(file);
    datastr = sprintf('%0.6f,',data);
    
    deviceObj = handles.device;
    groupObj = get(deviceObj, 'Arbitrarywaveformch1'); %144000 characters
    
    invoke(groupObj, 'customForm', '0.000063,0.000125,0.000188,0.000063,0.000125,0.000188');
    
    handle.asgdata.arbitrarydatach1 = data;
    guidata(hObject,handles);
else
    errordlg('Please provide existing file!');
end


end
% --- Executes on button press in Send.
function Send_Callback(hObject, eventdata, handles)
% hObject    handle to Send (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in trigg_imm_ch1.
function trigg_imm_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to trigg_imm_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

deviceObj = handles.device;

groupObj = get(deviceObj, 'Triggerch1');
invoke(groupObj, 'ImmediateTgigger');

parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch1')
        set(children(childid),'Value',0);
    end
end

handle.asgdata.ImmediateTgiggerch1 = 1;
guidata(hObject,handles);

% --- Executes during object creation, after setting all properties.
function trigg_source_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to trigg_source_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function IP_addr_Callback(hObject, eventdata, handles)
% hObject    handle to IP_addr (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of IP_addr as text
%        str2double(get(hObject,'String')) returns contents of IP_addr as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
handles.IP = get(hObject,'String');

guidata(hObject,handles);

% --- Executes during object creation, after setting all properties.
function IP_addr_CreateFcn(hObject, eventdata, handles)
% hObject    handle to IP_addr (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function connection_port_Callback(hObject, eventdata, handles)
% hObject    handle to connection_port (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of connection_port as text
%        str2double(get(hObject,'String')) returns contents of connection_port as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
handles.port = str2num(get(hObject,'String'));

% --- Executes during object creation, after setting all properties.
function connection_port_CreateFcn(hObject, eventdata, handles)
% hObject    handle to connection_port (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in connect_button.
function connect_button_Callback(hObject, eventdata, handles)
% hObject    handle to connect_button (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

%ping test
IP = handles.IP;
port = handles.port;


command=['ping -t 1 -c 1 ',IP];

[ping_status, ping_message] = system(command);
if (ping_status ~= 0)
    errordlg('Hardware not found (PING)');
    disp('Hardware not found (PING)');
    ping_message
else
    %Connect to Red Pitaya
    
    color = get(hObject,'BackgroundColor');
    interfaceObj = instrfind('Type', 'tcpip', 'RemoteHost',IP, 'RemotePort', port, 'Tag', '');
    
    
    parent = get(hObject,'parent'); %get parent object
    connection_status_tag = findobj(parent,'Tag','connection_status_tag');
    connectionstatus = get(connection_status_tag,'String');
    
    
    if(~strcmp(connectionstatus ,'Connected')) % connect
        
        %check for existing connections with the name 'fcngen' and remove them
        deviceObj = handles.device;
        if (size(deviceObj,2) > 1)
            %delete existing objects and reconnect
            disconnect(deviceObj);
            delete(interfaceObj);
            %set(hObject,'BackgroundColor',[0.9412 0.9412 0.9412]);
            handles.asgdata.connectionstate = 'offline';
            interfaceObj = instrfind('Type', 'tcpip', 'RemoteHost',IP, 'RemotePort', port, 'Tag', '');
        end
        
        % Create the TCPIP object if it does not exist
        % otherwise use the object that was found.
        if isempty(interfaceObj)
            interfaceObj = tcpip(IP, port);
        else
            fclose(interfaceObj);
            interfaceObj = interfaceObj(1);
        end
        %interfaceObj.Terminator ='CR/LF';
        deviceObj = icdevice('red_pitaya.mdd', interfaceObj);
        % Connect device object to hardware.
        interfaceObj.Terminator = 'CR/LF';
        
        interfaceObj.OutputBufferSize = 16384*64;
        interfaceObj.InputBufferSize = 16384*64;
        connect(deviceObj);
        %set(hObject,'BackgroundColor',[0 1 0]);
        
        handles.device = deviceObj;
        handles.asgdata.connectionstate = 'active';
        set(connection_status_tag,'String','Connected');
        set(hObject,'String','Disconnect');
    else % disconnect
        
        %deviceObj = get(handles.device)
        deviceObj = handles.device;
        disconnect(deviceObj);
        delete(interfaceObj);
        %set(hObject,'BackgroundColor',[0.9412 0.9412 0.9412]);
        handles.asgdata.connectionstate = 'offline';
        set(connection_status_tag,'String','Disconnected');
        set(hObject,'String','Connect');
    end
    
    handles.device = deviceObj;
    guidata(hObject,handles);
end



% --------------------------------------------------------------------
function Channel1_Callback(hObject, eventdata, handles)
% hObject    handle to Channel1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function Channel2_Callback(hObject, eventdata, handles)
% hObject    handle to Channel2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --------------------------------------------------------------------
function Settings_Callback(hObject, eventdata, handles)
% hObject    handle to Settings (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes on button press in radiobutton19.
function radiobutton19_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton19 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton19


% --- Executes on button press in radiobutton20.
function radiobutton20_Callback(hObject, eventdata, handles)
% hObject    handle to radiobutton20 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of radiobutton20


% --- Executes on button press in enable_ch2.
function enable_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to enable_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
% Execute device object function(s).
groupObj = get(deviceObj, 'Arbitrarywaveformch2');


if((get(hObject,'BackgroundColor') == [0,1,0]))
    set(hObject,'BackgroundColor',[0.9412    0.9412    0.9412]);
    invoke(groupObj, 'enable', 'OFF');
else
    set(hObject,'BackgroundColor','green');
    invoke(groupObj, 'enable', 'ON');
end
handles.asgdata.enablech2 = 'ON';
guidata(hObject,handles);

end



function frequency_input_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to frequency_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of frequency_input_ch2 as text
%        str2double(get(hObject,'String')) returns contents of frequency_input_ch2 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

frequencyinput = (str2double(get(hObject, 'String')));

if isnan(frequencyinput)
    set(hObject, 'String', 1000);
    errordlg('Input must be a number','Error');
end
frequency = frequencyinput * handles.asgdata.fr_prefixch2;
if (frequency > handles.asgdata.frequencyMAX)
    errordlg('Value must be lower than 0!');
    
elseif(frequency < 0)
    errordlg('Value must not be smaller than 0!');
end

frequencystr = num2str(frequency);
deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'frequency', frequencystr);

handles.asgdata.frequencych2 = frequencyinput;
guidata(hObject,handles);

end
% --- Executes during object creation, after setting all properties.
function frequency_input_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to frequency_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in Fr_prefix_ch2.
function Fr_prefix_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to Fr_prefix_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns Fr_prefix_ch2 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from Fr_prefix_ch2

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else
val = get(hObject, 'Value');
str = get(hObject, 'String');
switch str{val}
    case 'Hz' % User selects Hz
        handles.asgdata.fr_prefixch2  = 1e0;
    case 'KHz' % User selects kHz
        handles.asgdata.fr_prefixch2  = 1e3;
    case 'MHz' % User selects MHz
        handles.asgdata.fr_prefixch2  = 1e6;
end


frequency = handles.asgdata.frequencych2 * handles.asgdata.fr_prefixch2;
if (frequency > handles.asgdata.frequencyMAX)
    errordlg('Frequnecy excedes maximum value!');
    
elseif(frequency < handles.asgdata.frequencyMIN)
    errordlg('Frequnecy excedes minimum value!');
end
frequencystr = num2str(frequency);
deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'frequency', frequencystr);

guidata(hObject,handles);
end


% --- Executes during object creation, after setting all properties.
function Fr_prefix_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to Fr_prefix_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function offset_input_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to offset_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of offset_input_ch2 as text
%        str2double(get(hObject,'String')) returns contents of offset_input_ch2 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

offset = (str2double(get(hObject, 'String')));

if ((abs(handles.asgdata.amplitudech2)+abs(offset)) > 1)
    errordlg('Output amplitude can not be greater than 1V!');
end

if isnan(offset)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if (offset > 1)
    errordlg('Value must not be greater than 1!');
    
elseif(offset < -1)
    errordlg('Value must not be smaller than -1!');
end

offsetstr = num2str(offset);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Offset', offsetstr);

handles.asgdata.offsetch2 = offset;
guidata(hObject,handles);

end

% --- Executes during object creation, after setting all properties.
function offset_input_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to offset_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function duty_cycle_input_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to duty_cycle_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of duty_cycle_input_ch2 as text
%        str2double(get(hObject,'String')) returns contents of duty_cycle_input_ch2 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

dutycycle = (str2double(get(hObject, 'String')));

if isnan(dutycycle)
    set(hObject, 'String', 1);
    errordlg('Input must be a number','Error');
end

if (dutycycle > 100)
    errordlg('Duty Cycle can not be greater than 100%!');
    
elseif(dutycycle < 0)
    errordlg('Duty Cycle can not be lower than 0%!');
end

dutycyclestr = num2str(dutycycle);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'DutyCycle', dutycyclestr);

handles.asgdata.dutycyclech2 = dutycycle;
guidata(hObject,handles);
end
% --- Executes during object creation, after setting all properties.
function duty_cycle_input_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to duty_cycle_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function amplitude_input_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of amplitude_input_ch2 as text
%        str2double(get(hObject,'String')) returns contents of amplitude_input_ch2 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

amplitude = str2double(get(hObject, 'String'));

if ((abs(handles.asgdata.offsetch2)+abs(amplitude)) > 1)
    errordlg('Output amplitude can not be greater than 1V!');
end

if isnan(amplitude)
    set(hObject, 'String', 1);
    errordlg('Input must be a number','Error');
end

if (amplitude > 1)
    errordlg('Maximum value is 1!');
    
elseif(amplitude < 0)
    errordlg('Value can not be samller than 0!');
end

amplitudestr = num2str(amplitude);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Amplitude', amplitudestr);

handles.asgdata.amplitudech2 = amplitude;
guidata(hObject,handles);
end

% --- Executes during object creation, after setting all properties.
function amplitude_input_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function phase_input_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to phase_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of phase_input_ch2 as text
%        str2double(get(hObject,'String')) returns contents of phase_input_ch2 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

phase = str2double(get(hObject, 'String'));

if isnan(phase)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if (phase > 360)
    errordlg('Value must not be greater than 360!');
    
elseif(phase < -360)
    errordlg('Phase must not be smaller than -360!');
end

phasestr = num2str(phase);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Phase', phasestr);

handles.asgdata.phasech2 = phase;
guidata(hObject,handles);
end
% --- Executes during object creation, after setting all properties.
function phase_input_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to phase_input_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in load_btn_ch2.
function load_btn_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to load_btn_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

parent = get(hObject,'parent');
children = get(parent,'Children');
for childtag = 1:length(children)
    %get(children(childtag), 'Tag')
    if strcmp(get(children(childtag), 'Tag'),'arbitrary_file_insert_ch2')
        file =  get(children(childtag),'String');
    end
end
data=[];
if (exist(file)==2)
    data= dlmread(file);
    datastr = sprintf('%0.5f,',data);
    deviceObj = handles.device;
    groupObj = get(deviceObj, 'Arbitrarywaveformch2'); %144000 characters
    
    invoke(groupObj, 'CustomForm', datastr);
    
    handle.asgdata.arbitrarydatach2 = data;
    guidata(hObject,handles);
else
    errordlg('Please provide existing file!');
end
end
% --- Executes on button press in SINE_togg_ch2.
function SINE_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to SINE_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;

groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Waveform', 'SINE');

parent = get(hObject,'parent');
children = get(parent,'Children');



for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech2 = 'SINE';
guidata(hObject,handles);
end
% --- Executes on button press in TRI_togg_ch2.
function TRI_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to TRI_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Waveform', 'TRIANGLE');

parent = get(hObject,'parent');
children = get(parent,'Children');

for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech1 = 'TRIANGLE';
guidata(hObject,handles);
end
% --- Executes on button press in SQR_togg_ch2.
function SQR_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to SQR_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else
deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Waveform', 'SQUARE');


parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech1 = 'SQR';
guidata(hObject,handles);
end
% --- Executes on button press in SAWU_togg_ch2.
function SAWU_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to SAWU_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Waveform', 'SAWU');

parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech2 = 'SAWU';
guidata(hObject,handles);
end
% --- Executes on button press in SAWD_togg_ch2.
function SAWD_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to SAWD_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Waveform', 'SAWD');

parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.68  0.92 1]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'PWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end
handles.asgdata.typech2 = 'SAWD';
guidata(hObject,handles);
end
% --- Executes on button press in PWM_togg_ch2.
function PWM_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to PWM_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Waveform', 'PWM');


parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'PWM_togg_ch2')
        set(children(childid),'BackgroundColor',[ 0.68  0.92  1 ]);
    elseif strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech2 = 'PWM';
guidata(hObject,handles);
end

function edit24_Callback(hObject, eventdata, handles)
% hObject    handle to arbitrary_file_insert_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of arbitrary_file_insert_ch2 as text
%        str2double(get(hObject,'String')) returns contents of arbitrary_file_insert_ch2 as a double


% --- Executes during object creation, after setting all properties.
function arbitrary_file_insert_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to arbitrary_file_insert_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in ARB_togg_ch2.
function ARB_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to ARB_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch2');

invoke(groupObj, 'Waveform', 'ARBITRARY');


parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1 :  length(children)
    if strcmp(get(children(childid), 'Tag'),'ARB_togg_ch2')
        set(children(childid),'BackgroundColor',[0.68  0.98 0.68]);
    elseif strcmp(get(children(childid), 'Tag'),'PWM_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWD_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SAWU_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SQR_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'TRI_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    elseif strcmp(get(children(childid), 'Tag'),'SINE_togg_ch2')
        set(children(childid),'BackgroundColor',[0.9412    0.9412    0.9412]);
    end
    
end

handles.asgdata.typech2 = 'ARB';
guidata(hObject,handles);
end

function arbitrary_file_insert_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to arbitrary_file_insert_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of arbitrary_file_insert_ch2 as text
%        str2double(get(hObject,'String')) returns contents of arbitrary_file_insert_ch2 as a double
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
end



function edit26_Callback(hObject, eventdata, handles)
% hObject    handle to edit26 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit26 as text
%        str2double(get(hObject,'String')) returns contents of edit26 as a double


% --- Executes during object creation, after setting all properties.
function edit26_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit26 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on selection change in popupmenu5.
function popupmenu5_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns popupmenu5 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu5


% --- Executes during object creation, after setting all properties.
function popupmenu5_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu5 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit27_Callback(hObject, eventdata, handles)
% hObject    handle to edit27 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit27 as text
%        str2double(get(hObject,'String')) returns contents of edit27 as a double


% --- Executes during object creation, after setting all properties.
function edit27_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit27 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit28_Callback(hObject, eventdata, handles)
% hObject    handle to edit28 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit28 as text
%        str2double(get(hObject,'String')) returns contents of edit28 as a double


% --- Executes during object creation, after setting all properties.
function edit28_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit28 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit29_Callback(hObject, eventdata, handles)
% hObject    handle to edit29 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit29 as text
%        str2double(get(hObject,'String')) returns contents of edit29 as a double


% --- Executes during object creation, after setting all properties.
function edit29_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit29 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit30_Callback(hObject, eventdata, handles)
% hObject    handle to edit30 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of edit30 as text
%        str2double(get(hObject,'String')) returns contents of edit30 as a double


% --- Executes during object creation, after setting all properties.
function edit30_CreateFcn(hObject, eventdata, handles)
% hObject    handle to edit30 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in trigg_imm_ch2.
function trigg_imm_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to trigg_imm_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch2');
invoke(groupObj, 'ImmediateTrigger');
parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
        set(children(childid),'Value',0);
    end
end


handle.asgdata.ImmediateTgiggerch2 = 1;
guidata(hObject,handles);

% --- Executes on selection change in popupmenu10.
function popupmenu10_Callback(hObject, eventdata, handles)
% hObject    handle to popupmenu10 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns popupmenu10 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from popupmenu10


% --- Executes during object creation, after setting all properties.
function popupmenu10_CreateFcn(hObject, eventdata, handles)
% hObject    handle to popupmenu10 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function burst_period_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to burst_period_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of burst_period_ch2 as text
%        str2double(get(hObject,'String')) returns contents of burst_period_ch2 as a double
period = get(hObject,'String');
if(length(period) < 1)
    errordlg('Missing parameter!');
end
if (period < 1)
    error('Error value must be greater than 0!');
end
deviceObj = handles.device;
groupObj = get(deviceObj, 'Burstmodch2');
invoke(groupObj, 'InternalPeriod', period);
invoke(groupObj, 'Enabled', 'ON');
handles.asgdata.burst_periodch2 = period;
guidata(hObject,handles);

% --- Executes during object creation, after setting all properties.
function burst_period_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to burst_period_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function num_bursts_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of num_bursts_ch2 as text
%        str2double(get(hObject,'String')) returns contents of num_bursts_ch2 as a double
burstsnum= get(hObject,'String');
if(length(burstsnum) < 1)
    errordlg('Missing parameter!');
end
if (burstsnum < 1)
    error('Error value must be greater than 0!');
end

deviceObj = handles.device;
groupObj = get(deviceObj, 'Burstmodch2');

burstsnumstr = num2str(burstsnum);
invoke(groupObj, 'BurstCount', burstsnumstr);
invoke(groupObj, 'Enabled', 'ON');

handles.asgdata.num_burstsch2 = burstsnum;
guidata(hObject,handles);

% --- Executes during object creation, after setting all properties.
function num_bursts_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function number_ofcycles_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to number_ofcycles_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of number_ofcycles_ch2 as text
%        str2double(get(hObject,'String')) returns contents of number_ofcycles_ch2 as a double

cycles = get(hObject,'String');
if(length(cycles) < 1)
    errordlg('Missing parameter!');
end
cycles = str2double(get(hObject,'String'));

deviceObj = handles.device;
groupObj = get(deviceObj, 'Burstmodch2');

invoke(groupObj, 'Cycles', num2str(cycles));
invoke(groupObj, 'Enabled', 'ON');

handles.asgdata.burst_signal_periodesch2 = cycles;
guidata(hObject,handles);

% --- Executes during object creation, after setting all properties.
function number_ofcycles_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to number_ofcycles_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in enable_burst_togg_ch2.
function enable_burst_togg_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to enable_burst_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of enable_burst_togg_ch2

if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch2');
burstObj = get(deviceObj, 'Burstmodch2');


handles.asgdata.triggersourcech1 = 'INT';

parent = get(hObject,'parent');
children = get(parent,'Children');
%get(children(1))

if((get(hObject,'BackgroundColor') == [0.68  0.92 1]))
    set(hObject,'BackgroundColor',[0.9412    0.9412    0.9412]);
    invoke(burstObj, 'Enabled', 'OFF');
    invoke(groupObj, 'Source', 'INT');
    for childid = 1 :  length(children)
        if strcmp(get(children(childid), 'Tag'),'trigg_imm_ch2')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'number_ofcycles_ch2')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'num_bursts_ch2')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'burst_period_ch2')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'trigg_source_ch2')
            set(children(childid),'Enable','off');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch2')
            set(children(childid),'Enable','off');
            set(children(childid),'Value',0);
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch2')
            set(children(childid),'Enable','off');
            set(children(childid),'Value',0);
        elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
            set(children(childid),'Enable','off');
            set(children(childid),'Value',1);
        elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
            set(children(childid),'Enable','off');
            set(children(childid),'Value',0);
        end
        
    end
    
    %set fr again
    frequency = handles.asgdata.frequencych2 * handles.asgdata.fr_prefixch2;
    frequencystr = num2str(frequency);
    groupObj = get(deviceObj, 'Arbitrarywaveformch2');
    invoke(groupObj, 'frequency', frequencystr);
    
else
    set(hObject,'BackgroundColor',[0.68  0.92 1]);
    invoke(burstObj, 'Enabled', 'ON');
    invoke(groupObj, 'Source', 'INT');
    for childid = 1 :  length(children)
        if strcmp(get(children(childid), 'Tag'),'trigg_imm_ch2')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'number_ofcycles_ch2')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'num_bursts_ch2')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'burst_period_ch2')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'trigg_source_ch2')
            set(children(childid),'Enable','on');
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch2')
            set(children(childid),'Enable','on');
            set(children(childid),'Value',0);
        elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch2')
            set(children(childid),'Enable','on');    
            set(children(childid),'Value',0);
        elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
            set(children(childid),'Enable','on');
            set(children(childid),'Value',1);
        elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
            set(children(childid),'Enable','on');
            set(children(childid),'Value',0);
        end
        
    end
    %send default values
    groupObj = get(deviceObj, 'Burstmodch2');    
    invoke(groupObj, 'BurstCount', num2str(handles.asgdata.num_burstsch2));
    invoke(groupObj, 'Cycles', num2str(handles.asgdata.burst_signal_periodesch2));
    invoke(groupObj, 'InternalPeriod', num2str(handles.asgdata.burst_periodch2));
    
    parent = get(hObject,'parent');
    child = findobj(parent,'Tag','number_ofcycles_ch2');%find object
    set(child,'String',num2str(handles.asgdata.burst_signal_periodesch2)); %Set string to graphical element
    
    child = findobj(parent,'Tag','num_bursts_ch2');%find object
    set(child,'String',num2str(handles.asgdata.num_burstsch2)); %Set string to graphical element
    
    child = findobj(parent,'Tag','burst_period_ch2');%find object
    set(child,'String',num2str(handles.asgdata.burst_periodch2)); %Set string to graphical element
    
end

handles.asgdata.EnableBurstch2 = 1;
guidata(hObject,handles);
end

% --- Executes on selection change in trigg_source_ch1.
function trigg_source_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to trigg_source_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = cellstr(get(hObject,'String')) returns trigg_source_ch1 contents as cell array
%        contents{get(hObject,'Value')} returns selected item from trigg_source_ch1


% --- Executes on button press in trigger_gated_radio_ch1.
function trigger_gated_radio_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_gated_radio_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_gated_radio_ch1
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch1');
burstObj = get(deviceObj, 'Burstmodch1');
invoke(groupObj, 'Source', 'GATED');
invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
        set(children(childid),'Value',1);
    end
end


handles.asgdata.triggersourcech1 = 'GATED';
guidata(hObject,handles);

% --- Executes on button press in trigger_internal_radio_ch1.
function trigger_internal_radio_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_internal_radio_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_internal_radio_ch1
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch1');
burstObj = get(deviceObj, 'Burstmodch1');
invoke(groupObj, 'Source', 'INT');
invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
        set(children(childid),'Value',1);
    end
end


handles.asgdata.triggersourcech1 = 'INT';
guidata(hObject,handles);


% --- Executes on button press in trigger_external_N_radio_ch2.
function trigger_external_N_radio_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_external_N_radio_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_external_N_radio_ch2
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch2');
burstObj = get(deviceObj, 'Burstmodch2');
invoke(groupObj, 'Source', 'EXT_NE');
invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');

for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch2')
        set(children(childid),'Value',1);
    end
end



handles.asgdata.triggersourcech2 = 'EXT_NE';
guidata(hObject,handles);

% --- Executes on button press in trigger_gated_radio_ch2.
function trigger_gated_radio_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_gated_radio_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_gated_radio_ch2
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch2');
burstObj = get(deviceObj, 'Burstmodch2');
invoke(groupObj, 'Source', 'GATED');
invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
        set(children(childid),'Value',1);
    end
end

handles.asgdata.triggersourcech2 = 'GATED';
guidata(hObject,handles);

% --- Executes on button press in trigger_internal_radio_ch2.
function trigger_internal_radio_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_internal_radio_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_internal_radio_ch2
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch2');
burstObj = get(deviceObj, 'Burstmodch2');
invoke(groupObj, 'Source', 'INT');
invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');
for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
        set(children(childid),'Value',1);
    end
end

handles.asgdata.triggersourcech2 = 'INT';
guidata(hObject,handles);


% --- Executes on button press in trigger_external_N_radio_ch2.
function radiobutton26_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_external_N_radio_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_external_N_radio_ch2
%

% --- Executes on button press in trigger_external_N_radio_ch1.
function trigger_external_N_radio_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_external_N_radio_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_external_N_radio_ch1
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch1');
burstObj = get(deviceObj, 'Burstmodch1');
invoke(groupObj, 'Source', 'EXT_NE');
invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');
%get(children(1))

for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch1')
        set(children(childid),'Value',1);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch1')
        set(children(childid),'Value',0);
    end
end

handles.asgdata.triggersourcech2 = 'EXT_NE';
guidata(hObject,handles);


% --- Executes during object creation, after setting all properties.
function frequency_text_CreateFcn(hObject, eventdata, handles)
% hObject    handle to frequency_text (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called



function num_bursts_ch12_Callback(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch12 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of num_bursts_ch12 as text
%        str2double(get(hObject,'String')) returns contents of num_bursts_ch12 as a double



% --- Executes during object creation, after setting all properties.
function num_bursts_ch12_CreateFcn(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch12 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes during object creation, after setting all properties.
function num_bursts_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function num_bursts_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to num_bursts_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of num_bursts_ch1 as text
%        str2double(get(hObject,'String')) returns contents of num_bursts_ch1 as a double
deviceObj = handles.device;
if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
number_of_bursts = str2double(get(hObject,'String'));
if (number_of_bursts > handles.asgdata.num_bursts_MAX)
    errordlg('Maximum number of bursts is 50000');
end

number_of_burstsstr = get(hObject,'String');
groupObj = get(deviceObj, 'Burstmodch1');

invoke(groupObj, 'BurstCount', number_of_burstsstr);
invoke(groupObj, 'Enabled', 'ON');

handles.asgdata.num_burstsch1 = number_of_bursts;
guidata(hObject,handles);


function amplitude_input_ch11_Callback(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch11 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of amplitude_input_ch11 as text
%        str2double(get(hObject,'String')) returns contents of amplitude_input_ch11 as a double
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

amplitude = str2double(get(hObject, 'String'));
if isnan(amplitude)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if ((abs(handles.asgdata.offsetch1)+abs(amplitude)) > 1)
    errordlg('Output amplitude can not be greater than 1V!');
end
amplitude = (str2double(get(hObject, 'String')));

if isnan(amplitude)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if (amplitude > 1)
    errordlg('Amplitude excedes maximum value!');
    
elseif(amplitude <= 0)
    errordlg('Amplitude excedes minimum value!');
end

amplitudestr = num2str(amplitude);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'Amplitude', amplitudestr);

handles.asgdata.amplitudech1 = amplitude;
guidata(hObject,handles);
end

% % --- Executes during object creation, after setting all properties.
% function amplitude_input_ch1_CreateFcn(hObject, eventdata, handles)
% % hObject    handle to amplitude_input_ch11 (see GCBO)
% % eventdata  reserved - to be defined in a future version of MATLAB
% % handles    empty - handles not created until after all CreateFcns called
% 
% % Hint: edit controls usually have a white background on Windows.
% %       See ISPC and COMPUTER.
% if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
%     set(hObject,'BackgroundColor','white');
% end



function amplitude_input_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of amplitude_input_ch1 as text
%        str2double(get(hObject,'String')) returns contents of amplitude_input_ch1 as a double

if(length(get(hObject,'String')) < 1)
    errordlg('Missing parameter!');
end
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

amplitude = str2double(get(hObject, 'String'));
if isnan(amplitude)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if ((abs(handles.asgdata.offsetch1)+abs(amplitude)) > 1)
    errordlg('Output amplitude can not be greater than 1V!');
end
amplitude = (str2double(get(hObject, 'String')));

if isnan(amplitude)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if (amplitude > 1)
    errordlg('Amplitude excedes maximum value!');
    
elseif(amplitude <= 0)
    errordlg('Amplitude excedes minimum value!');
end

amplitudestr = num2str(amplitude);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'Amplitude', amplitudestr);

handles.asgdata.amplitudech1 = amplitude;
guidata(hObject,handles);
end

% --- Executes during object creation, after setting all properties.
function amplitude_input_ch1_CreateFcn(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end



function edit55_Callback(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of amplitude_input_ch1 as text
%        str2double(get(hObject,'String')) returns contents of amplitude_input_ch1 as a double
if (~strcmp(handles.asgdata.connectionstate,'active'))
    errordlg(handles.asgdata.connectmessage);
else

amplitude = str2double(get(hObject, 'String'));
if isnan(amplitude)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if ((abs(handles.asgdata.offsetch1)+abs(amplitude)) > 1)
    errordlg('Output amplitude can not be greater than 1V!');
end
amplitude = (str2double(get(hObject, 'String')));

if isnan(amplitude)
    set(hObject, 'String', 0);
    errordlg('Input must be a number','Error');
end

if (amplitude > 1)
    errordlg('Amplitude excedes maximum value!');
    
elseif(amplitude <= 0)
    errordlg('Amplitude excedes minimum value!');
end

amplitudestr = num2str(amplitude);

deviceObj = handles.device;
groupObj = get(deviceObj, 'Arbitrarywaveformch1');

invoke(groupObj, 'Amplitude', amplitudestr);

handles.asgdata.amplitudech1 = amplitude;
guidata(hObject,handles);
end

% --- Executes during object creation, after setting all properties.
function edit55_CreateFcn(hObject, eventdata, handles)
% hObject    handle to amplitude_input_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc && isequal(get(hObject,'BackgroundColor'), get(0,'defaultUicontrolBackgroundColor'))
    set(hObject,'BackgroundColor','white');
end


% --- Executes on button press in trigger_external_P_radio_ch2.
function trigger_external_P_radio_ch2_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_external_P_radio_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_external_P_radio_ch2
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch2');
burstObj = get(deviceObj, 'Burstmodch2');
invoke(groupObj, 'Source', 'EXT_PE');
invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');
%get(children(1))

for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch2')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch2')
        set(children(childid),'Value',1);
    end
end

handles.asgdata.triggersourcech2 = 'EXT_PE';
guidata(hObject,handles);

% --- Executes on button press in trigger_external_P_radio_ch1.
function trigger_external_P_radio_ch1_Callback(hObject, eventdata, handles)
% hObject    handle to trigger_external_P_radio_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_external_P_radio_ch1
% hObject    handle to trigger_external_N_radio_ch1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of trigger_external_N_radio_ch1
deviceObj = handles.device;
groupObj = get(deviceObj, 'Triggerch1');
burstObj = get(deviceObj, 'Burstmodch1');
invoke(groupObj, 'Source', 'EXT_PE');

invoke(burstObj, 'Enabled', 'ON');
parent = get(hObject,'parent');
children = get(parent,'Children');
%get(children(1))

for childid = 1:length(children)
    if strcmp(get(children(childid), 'Tag'),'trigger_external_P_radio_ch1')
        set(children(childid),'Value',1);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_internal_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_gated_radio_ch1')
        set(children(childid),'Value',0);
    elseif strcmp(get(children(childid), 'Tag'),'trigger_external_N_radio_ch1')
        set(children(childid),'Value',0);
    end
end

handles.asgdata.triggersourcech2 = 'EXT_PE';
guidata(hObject,handles);


% --- Executes when figure1 is resized.
function figure1_ResizeFcn(hObject, eventdata, handles)
% hObject    handle to figure1 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)


% --- Executes during object creation, after setting all properties.
function enable_burst_togg_ch2_CreateFcn(hObject, eventdata, handles)
% hObject    handle to enable_burst_togg_ch2 (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called
