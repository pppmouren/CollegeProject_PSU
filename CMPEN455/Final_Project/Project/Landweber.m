function x = Landweber(g, H, iter, stepSize)
% g is the blurred image
% H is the blur function called PSF
% stepSize is a constant that controls the sharpening quantity
% iter is the number of the iteration
% Equation: f(n+1) = f(n) + stepSize * H^T* (g - H * f(n))

% Initialization
x = g;

% Landweber iteration
for i = 1:iter
    conv = conv2(x, H, 'same');
    residual = g - conv;
    conv_2 = conv2(residual, H', 'same');
    x = x + stepSize .* conv_2;
end

