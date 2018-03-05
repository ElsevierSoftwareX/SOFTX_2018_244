% Compare voltage and current of c++ simulation with voltage and currents
% from PLECS simulation
clc
clear all
%% read PLECS results
Results_Reference= csvread('../../../vsa/Results/TestExciterAndTurbine/Simulink/Voltages_and_currents.csv');
l_Ref = length(Results_Reference);
%Results_Reference = Results_Reference(1:l_Ref,:);
%Te_Reference = csvread('../../../vsa/Results/ABCFault/Simulink/Te.csv'); 
omega_Reference = csvread('../../../vsa/Results/TestExciterAndTurbine/Simulink/omega.csv'); 
vt_Reference = csvread('../../../vsa/Results/TestExciterAndTurbine/Simulink/vt.csv'); 
%theta_PLECS = csvread('../../../vsa/Results/SynGenDq_ABCFault/Sim-0.81113286269894136ulink_PLECS/SynGenDqEmt_ABCFault_300M_Simulink/theta.csv'); 
%% read results from c++ simulation
VoltageVector = csvread('../../../vsa/Results/TestExciterAndTurbine/DPsim/EMT/VBR/EMT_SynchronGenerator_VBR_LeftVector.csv',1);
%CurrentVector = csvread('../../../vsa/Results/MultimachineTest/DPsim/EMT_SynchronGenerator_VBR_RightVector.csv',1);
Log_SynGen = csvread('../../../vsa/Results/TestExciterAndTurbine/DPsim/EMT/VBR/SynGen_gen.csv',1);
CurrentVector = Log_SynGen(:,1:4);
 %% Plot
figure(1)
hold off
plot(VoltageVector(:,1),VoltageVector(:,2));
hold on
plot(Results_Reference(:,1),Results_Reference(:,2),'--');

title('Voltage Phase a');
legend('va DPSim','va Reference');

figure(2)
hold off
plot(VoltageVector(:,1), VoltageVector(:,3));
hold on
plot(Results_Reference(:,1),Results_Reference(:,3),'--');

title('Voltage Phase b');
legend('vb DPSim','vb Reference');

figure(3)
hold off
plot(VoltageVector(:,1),VoltageVector(:,4));
hold on
plot(Results_Reference(:,1),Results_Reference(:,4),'--');

title('Voltage Phase c');
legend('vc DPSim','vc Reference');

figure(4)
hold off
plot(CurrentVector(:,1),-CurrentVector(:,2));
hold on
plot(Results_Reference(:,1),Results_Reference(:,5),'--');
title('Current phase a');
legend('ia DPSim','ia Reference');
xlabel('Time [s]');
ylabel('Current [A]');

figure(5)
hold off
plot(CurrentVector(:,1),-CurrentVector(:,3));
hold on
plot(Results_Reference(:,1),Results_Reference(:,6),'--');
title('Current phase b');
legend('ib DPSim','ib Reference');

figure(6)
hold off
plot(CurrentVector(:,1),-CurrentVector(:,4));
hold on
plot(Results_Reference(:,1),Results_Reference(:,7),'--');
title('Current phase c');
legend('ic DPSim','ic Simulink');

% figure(3)
% hold off
% plot(Log_SynGen(:,1),Log_SynGen(:,2));
% hold on
% plot(Log_SynGen(:,1),Log_SynGen(:,3));
% plot(Log_SynGen(:,1),Log_SynGen(:,4));
% title ('Fluxes');
% legend('q','d','fd');
% 
% figure(4)
% hold off
% plot(Log_SynGen(:,1),Log_SynGen(:,5));
% hold on
% plot(Log_SynGen(:,1),Log_SynGen(:,6));
% plot(Log_SynGen(:,1),Log_SynGen(:,7));
% title ('dq voltages');
% legend('q','d','fd');
% 
% figure(5)
% hold off
% plot(Log_SynGen(:,1),Log_SynGen(:,8));
% hold on
% plot(Log_SynGen(:,1),Log_SynGen(:,9));6
% plot(Log_SynGen(:,1),Log_SynGen(:,10));
% title ('dq currents');
% legend('q','d','fd');
% 
figure(7)
hold off
plotomega1 = plot(Log_SynGen(:,1),Log_SynGen(:,6));
hold on
plotomega2 = plot(Results_Reference(:,1),omega_Reference*2*pi*60);
%title('Rotor speed');
legend('\omega DPSim','\omega Reference');
xlabel('Time [s]');
ylabel('\omega [rad/s]');

set(plotomega1,'LineWidth',2);
set(plotomega2,'LineWidth',2);

figure(8)
hold off
plotvt1 = plot(Log_SynGen(:,1),Log_SynGen(:,8));
hold on
plotvt2 = plot(Results_Reference(:,1),vt_Reference);
%title('vt');
legend('Terminal Voltage DPSim','Terminal Voltage Reference');
xlabel('Time [s]');
ylabel('Terminal Voltage [V]');

set(plotvt1,'LineWidth',2);
set(plotvt2,'LineWidth',2);


% 
% figure(8)
% hold off
% plot(Log_SynGen(:,1),Log_SynGen(:,7));
% hold on
% plot(Results_Reference(:,1),-Te_Reference); 
% title('Electrical Torque');
%  legend('Te DPSim','Te Reference');




%% Calculate and display error
%Cut Current and Voltage vector to get steady state results
l=length(CurrentVector);
l_new=round(0.1/20*l_Ref);

% if l == l_Ref
%     CurrentVector_SteadyState = CurrentVector(1:l_new,2);
%     CurrentVector_Fault = CurrentVector(l_new+1:2*l_new-1,2);
% else
%     s = round(l_Ref/l);
%     CurrentVector_interpolated = interp(CurrentVector(:,2),s);
%     CurrentVector_SteadyState = CurrentVector_interpolated(1:l_new,:);
%     CurrentVector_Fault = CurrentVector_interpolated(l_new+1:2*l_new-1,:);  
% end  

if l == l_Ref
    CurrentVector_SteadyState = CurrentVector(1:l_new,2);
    CurrentVector_Fault = CurrentVector(l_new+1:l_Ref,2);
else
    s = round(l_Ref/l);
    CurrentVector_interpolated = interp(CurrentVector(:,2),s);
    CurrentVector_SteadyState = CurrentVector_interpolated(1:l_new,:);
    CurrentVector_Fault = CurrentVector_interpolated(l_new+1:l_Ref,:);  
end  
    
%Cut Reference Results to get results before fault clearing
Reference_SteadyState = Results_Reference(1:l_new,5);
Reference_Fault = Results_Reference(l_new+1:l_Ref,5);

% if(l_Ref == l)
% Reference_SteadyState = Results_Reference(1:l_new,5);
% Reference_Fault = Results_Reference(l_new+1:2*l_new-1,5);
% else
% ReferenceCurrent = Results_Reference(:,5);
% ReferenceCurrent_resampled = resample(ReferenceCurrent,l,length(ReferenceCurrent));
% Reference_SteadyState = ReferenceCurrent_resampled(1:l_new,:);
% Reference_Fault = ReferenceCurrent_resampled(l_new+1:2*l_new-1,:);
% end

% figure(8)
% hold off
% plot(Results_Reference(1:l_new,1),-CurrentVector_SteadyState)
% hold on
% plot(Results_Reference(1:l_new,1),Reference_SteadyState)
% 
% figure(9)
% hold off
% plot(Results_Reference(l_new+1:2*l_new-1,1),-CurrentVector_Fault)
% hold on
% plot(Results_Reference(l_new+1:2*l_new-1,1),Reference_Fault)

Peak_Ref_SS = 10209;
%Peak_Ref_fault = 14650;
%Peak_Ref_fault = max(Reference_Fault);
Peak_Ref_fault = rms(Reference_Fault)*sqrt(2);

% % Current phase a steady state
Dif_SS = abs(-CurrentVector_SteadyState - Reference_SteadyState);
[MaxDif_SS,i1] = max(Dif_SS);
err_SS = sqrt(immse(-CurrentVector_SteadyState,Reference_SteadyState));
disp(['Maximum Error ia steady state: ', num2str(MaxDif_SS), ' A']);
disp(['Root Mean-squared error ia steady state: ', num2str(err_SS), ' A']);
disp(['Maximum Error ia steady state: ', num2str(100*MaxDif_SS/Peak_Ref_SS), ' %']);
disp(['Root Mean-squared error va steady state: ', num2str(100*err_SS/Peak_Ref_SS), ' %']);

% % Current phase a fault
Dif_Fault = abs(-CurrentVector_Fault - Reference_Fault);
[MaxDif_Fault,i2] = max(Dif_Fault);
err_Fault = sqrt(immse(-CurrentVector_Fault,Reference_Fault));
disp(['Maximum Error ia Fault: ', num2str(MaxDif_Fault), ' A']);
disp(['Root Mean-squared error ia Fault: ', num2str(err_Fault), ' A']);
disp(['Maximum Error ia Fault: ', num2str(100*MaxDif_Fault/Peak_Ref_fault), ' %']);
disp(['Root Mean-squared error ia Fault: ', num2str(100*err_Fault/Peak_Ref_fault), ' %']);

%% Calculate avarage step time
StepTimeVector = Log_SynGen(:,7);
disp(['Avarage step time for generator: ', num2str(mean(StepTimeVector)*1000), ' ms']);