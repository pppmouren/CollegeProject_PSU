function outarray = apply_fullconnect(inarray,filterbank, biasvals)
% 
% Input:  inarray, array of size 4x4x10
%         filterbank, array of size 4x4x10x10
%         biasvals, array of size 1x10
% Output: outarray, array of size 1x1x10

% Init the outarray
outarray = zeros(1,1,size(filterbank,4));

% Implement full connect algorithm
for dim = 1:size(filterbank, 4)
    sum = 0;
    for k = 1:size(inarray, 3)
        for i = 1:size(inarray, 1)
            for j = 1:size(inarray, 2)
                sum = sum + inarray(i,j,k)*filterbank(i,j,k,dim);
            end
        end
    end

    % add bias 
    sum = sum + biasvals(dim);

    %store to output array
    outarray(1,1,dim) = sum;
end

end