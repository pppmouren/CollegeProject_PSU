function x = ALM(g, H, iter, stepSize)
% g is the blurred image
% H is the blur function called PSF
% stepSize is a constant that controls the sharpening quantity
% iter is the number of the iteration

% Initialization
beta = stepSize;
[m,n] = size(g);
x = zeros(m,n);
stackedVecG = g(:);
stackedVecX1 = x(:);
%X1 = x;

% first iter init
firstConv1 = conv2(stackedVecX1, H, 'same');
firstRes = stackedVecG - firstConv1;
firstConv2 = conv2(firstRes, H', 'same');
stackedVecX1 = stackedVecX1 + beta * firstConv2;

% second iter init
secondConv1 = conv2(stackedVecX1, H, 'same');
secondRes = stackedVecG - secondConv1;
secondConv2 = conv2(secondRes, H', 'same');
stackedVecX2 = stackedVecX1 + beta * secondConv2;

% get alpha
if norm(gradient(stackedVecX1)) ~= 0
    alpha = norm(gradient(stackedVecX2)) / norm(gradient(stackedVecX1));
else
    alpha = beta;
end

% Looping 
for i = 3:iter
    loopConv1 = conv2( stackedVecX2, H, 'same');
    loopres = stackedVecG - loopConv1;
    loopConv2 = conv2(loopres, H', 'same');
    % assign X1 with X2
    stackedVecX1 = stackedVecX2;
    stackedVecX2 = stackedVecX2 + max(beta, alpha) * loopConv2;
    
    % update the stepSize 
    if norm(gradient(stackedVecX1)) ~= 0
        alpha = norm(gradient(stackedVecX2)) / norm(gradient(stackedVecX1));
    else
        alpha = beta;
    end
end
% assign output
x = reshape(stackedVecX2, m, n);

