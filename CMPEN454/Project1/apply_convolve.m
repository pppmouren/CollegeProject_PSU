function outarray = apply_convolve(inarray,filterbank,biasvals)
% Convolution filter function
% 
% Input:   inarray, array of size 32x32x3, 16x16x10, 8x8x10
%          filterbanks, array of size 3x3x10x10 or 3x3x10x10
%          biasvals, 1x10
% Output:  outarray, array of size 32x32x10

% Init the outarray
outarray = zeros(size(inarray,1), size(inarray,2), size(filterbank,4));

% Implement convolution
for i = 1:size(filterbank,4)
    for j = 1: size(inarray,3)
        temp_arr = imfilter(inarray(:,:,j),filterbank(:,:,j,i), zeros,'conv');
        outarray(:,:,i) = outarray(:,:,i)+temp_arr;
    end
    
    % Add the bias value
    outarray(:,:,i) = outarray(:,:,i) + biasvals(i);
end
end

