%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
% Project:     Object Detectiom (CMPEN454 Project 1)
%
% Overview:    The goal of this project will be to implement the forward 
%              pass of a Convolutional Neural Network for performing object 
%              detection. We will see how our ideas of applying image filters
%              can be (and have been) combined to construct powerful object 
%              detection algorithms in images. 
%
% Authors:    Jianwu Tan, Jie Chen, Qiaoxu Cui, Xuhong Lin
% Instructor: Prof. Kenneth D. Czuprynski
% Date:       09/25/2023
% Attribute: 
%             Main Routine:         Everyone
%             apply_imnormalize.m:  JianWu Tan
%             apply_relu.m:         JianWu Tan
%             apply_maxpool.m:      Qiaoxu Cui
%             apply_convolve.m:     Xuhong Lin
%             apply_fullconnect.m:  Xuhong Lin
%             apply_softmax.m:      Jie Chen
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
load 'cifar10testdata.mat'
load CNNparameters.mat
load debuggingTest.mat


result = zeros(10,1000);
% Create confusion matrix
confusion_max = zeros(10,10);
for classindex = 1:10
    inds = find(trueclass == classindex);
    
    % get the image class by class, each class has 1000 pictures
    for i = 1:1000
        % get image and show image
        imrgb = imageset(:,:,:, inds(i));
        %figure; imagesc(imrgb); truesize(gcf,[64 64]);

        % convert imtgb from unit8 to double to do normalization
        imrgb = double(imrgb);

        %apply forward CNN layer
        % 1
        imrgb = apply_imnormalize(imrgb);
        % 2
        imrgb = apply_convolve(imrgb,filterbanks{2}, biasvectors{2});
        % 3
        imrgb = apply_relu(imrgb);
        % 4
        imrgb = apply_convolve(imrgb,filterbanks{4}, biasvectors{4});
        % 5
        imrgb = apply_relu(imrgb);
        % 6
        imrgb = apply_maxpool(imrgb);
        % 7 
        imrgb = apply_convolve(imrgb,filterbanks{7}, biasvectors{7});
        % 8
        imrgb = apply_relu(imrgb);
        % 9
        imrgb = apply_convolve(imrgb,filterbanks{9}, biasvectors{9});
        % 10
        imrgb = apply_relu(imrgb);
        % 11
        imrgb = apply_maxpool(imrgb);
        % 12
        imrgb = apply_convolve(imrgb, filterbanks{12}, biasvectors{12});
        %13
        imrgb = apply_relu(imrgb);
        %14
        imrgb = apply_convolve(imrgb, filterbanks{14}, biasvectors{14});
        %15
        imrgb = apply_relu(imrgb);
        %16
        imrgb = apply_maxpool(imrgb);
        %17
        imrgb = apply_fullconnect(imrgb,filterbanks{17}, biasvectors{17});
        %18 
        imrgb = apply_softmax(imrgb);
        %disp(imrgb);

        % get the maximum value and corresponding index from output of our
        % network
        [max_val, max_index] = max(imrgb);

        % put max_index to result matrix
        result(classindex,i) = max_index;

        % increment confusion matrix 
        confusion_max(classindex,max_index) = confusion_max(classindex,max_index)+1;
    end
end

% get accuracy denominator
denominador = sum(sum(confusion_max));
% get accuracy molecular
molecular = 0;
for t = 1:10
    molecular = molecular + confusion_max(t,t);
end
% calculate accuracy and print it out
accuracy = molecular/denominador;
fprintf("The accuracy of the network will be %0.4f\n", accuracy);



%%
% Test Debug file
% In each layer, I load the imrgb as the correct answer from previous layer
%   from layerResults.mat, and then I run the functions we wrote. I diaplay
%   the imrbg after apply to the function, we need to go the corresponding
%   answer from layerResults.mat to check our functionality.


load debuggingTest.mat
load 'cifar10testdata.mat'
load CNNparameters.mat
%loading this file defines imrgb and layerResult
figure; imagesc(imrgb);truesize(gcf,[64,64]);
for d =1:length(layerResults)
    result_debug = layerResults{d};
    fprintf('layer %d output is size %d x %d x %d\n', ...
        d,size(result_debug,1),size(result_debug,2),size(result_debug,3));
end
classprobvec = squeeze(layerResults{end});
[maxprob,maxclass]= max(classprobvec);
fprintf('estimated class is %s with probaility %.4f\n', ...
    classlabels{maxclass},maxprob);

imrgb = double(imrgb);
% 1
imrgb = apply_imnormalize(imrgb);
disp("\nLayer 1\n");
disp(imrgb);

% 2
% load the correct answer from pervious answer
imrgb = layerResults{1};
% apply the function we write
imrgb = apply_convolve(imrgb,filterbanks{2}, biasvectors{2});
% diaplat what we get and go to layerResult.mat to compare result
disp("\nLayer 2\n");
disp(imrgb);

% 3
imrgb = layerResults{2};
imrgb = apply_relu(imrgb);
disp("\nLayer 3\n");
disp(imrgb);

% 4
imrgb = layerResults{3};
imrgb = apply_convolve(imrgb,filterbanks{4}, biasvectors{4});
disp("\nLayer 4\n");
disp(imrgb);

% 5
imrgb = layerResults{4};
imrgb = apply_relu(imrgb);
disp("\nLayer 5\n");
disp(imrgb);

% 6
imrgb = layerResults{5};
imrgb = apply_maxpool(imrgb);
disp("\nLayer 6\n");
disp(imrgb);

% 7
imrgb = layerResults{6};
imrgb = apply_convolve(imrgb,filterbanks{7}, biasvectors{7});
disp("\nLayer 7\n");
disp(imrgb);

% 8
imrgb = layerResults{7};
imrgb = apply_relu(imrgb);
disp("\nLayer 8\n");
disp(imrgb);

% 9
imrgb = layerResults{8};
imrgb = apply_convolve(imrgb,filterbanks{9}, biasvectors{9});
disp("\nLayer 9\n");
disp(imrgb);

% 10
imrgb = layerResults{9};
imrgb = apply_relu(imrgb);
disp("\nLayer 10\n");
disp(imrgb);

% 11
imrgb = layerResults{10};
imrgb = apply_maxpool(imrgb);
disp("\nLayer 11\n");
disp(imrgb);

% 12
imrgb = layerResults{11};
imrgb = apply_convolve(imrgb, filterbanks{12}, biasvectors{12});
disp("\nLayer 12\n");
disp(imrgb);

%13
imrgb = layerResults{12};
imrgb = apply_relu(imrgb);
disp("\nLayer 13\n");
disp(imrgb);

%14
imrgb = layerResults{13};
imrgb = apply_convolve(imrgb, filterbanks{14}, biasvectors{14});
disp("\nLayer 14\n");
disp(imrgb);

%15
imrgb = layerResults{14};
imrgb = apply_relu(imrgb);
disp("\nLayer 15\n");
disp(imrgb);

%16
imrgb = layerResults{15};
imrgb = apply_maxpool(imrgb);
disp("\nLayer 16\n");
disp(imrgb);

%17
imrgb = layerResults{16};
imrgb = apply_fullconnect(imrgb,filterbanks{17}, biasvectors{17});
disp("\nLayer 17\n");
disp(imrgb);

%18
imrgb = layerResults{17};
imrgb = apply_softmax(imrgb);
disp("\nLayer 18\n");
disp(imrgb);



%%
% This section does not apply the correct answer from previous layer. This 
% section directly get the final result.

load debuggingTest.mat
load 'cifar10testdata.mat'
load CNNparameters.mat
load debuggingTest.mat
%loading this file defines imrgb and layerResult
figure; imagesc(imrgb);truesize(gcf,[64,64]);
for d =1:length(layerResults)
    result_debug = layerResults{d};
    fprintf('layer %d output is size %d x %d x %d\n', ...
        d,size(result_debug,1),size(result_debug,2),size(result_debug,3));
end
classprobvec = squeeze(layerResults{end});
[maxprob,maxclass]= max(classprobvec);
fprintf('estimated class is %s with probaility %.4f\n', ...
    classlabels{maxclass},maxprob);

imrgb = double(imrgb);

%apply forward CNN layer
% 1
imrgb = apply_imnormalize(imrgb);
% 2
imrgb = apply_convolve(imrgb,filterbanks{2}, biasvectors{2});
% 3
imrgb = apply_relu(imrgb);
% 4
imrgb = apply_convolve(imrgb,filterbanks{4}, biasvectors{4});
% 5
imrgb = apply_relu(imrgb);
% 6
imrgb = apply_maxpool(imrgb);
% 7
imrgb = apply_convolve(imrgb,filterbanks{7}, biasvectors{7});
% 8
imrgb = apply_relu(imrgb);
% 9
imrgb = apply_convolve(imrgb,filterbanks{9}, biasvectors{9});
% 10
imrgb = apply_relu(imrgb);
% 11
imrgb = apply_maxpool(imrgb);
% 12
imrgb = apply_convolve(imrgb, filterbanks{12}, biasvectors{12});
%13
imrgb = apply_relu(imrgb);
%14
imrgb = apply_convolve(imrgb, filterbanks{14}, biasvectors{14});
%15
imrgb = apply_relu(imrgb);
%16
imrgb = apply_maxpool(imrgb);
%17
imrgb = apply_fullconnect(imrgb,filterbanks{17}, biasvectors{17});
%18
imrgb = apply_softmax(imrgb);
% display the final result
disp("imrgb = ");
disp(imrgb);
[max_val, max_index] = max(imrgb);
fprintf("Max value of result = %.4f, Max index = %d", max_val, max_index);






