function outarray = apply_imnormalize(inarray)
% 
% Input:  inarray, array of size 32x32x3
%        
% Output: outarray, array of size 1x1x10

outarray = inarray / 255 - 0.5;
end
