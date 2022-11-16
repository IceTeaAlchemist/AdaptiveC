function [coeff, iter] = GaussNewton3(x,y,beta0,lb,ub)
% Set function tolerance, this is the maximum change for convergence
% condition.
tol =0.0001;
% Set the maximum number of iterations the program will take.
iter_max = 26;
n = length(x);
% Set up the initial guess.
coeff = beta0;
f = zeros(n,1);
j = zeros(n,3);
d = zeros(1,n);


% Begin looping.
for iter = 1:iter_max
    % Set coefficient guesses.
    a = coeff(1);
    b = coeff(2);
    c = coeff(3);
    % Create the Jacobian.
    for i = 1:n
        f(i,1) = a*exp(-(x(i) - b)^2/(2*c^2));
        j(i,1) = exp(-(x(i) - b)^2/(2*c^2));
        j(i,2) = a*(x(i)-b)*exp(-(x(i) - b)^2/(2*c^2))/c^2;
        j(i,3) = a*(x(i)-b)^2*exp(-(x(i) - b)^2/(2*c^2))/c^3;
        d(i) = y(i) - f(i);
    end
    % Damp the Jacobian to avoid poor conditioning and add it.
    da = (j'*j + 0.00001*eye(size(j'*j)))\(j'*d');
    % Iterate.
    coeff = coeff +da';
    
    % Invoke boundary conditions.
    if(coeff(1) < lb(1))
        coeff(1) = lb(1);
    end
    if(coeff(2) < lb(2))
        coeff(2) = lb(2);
    end
    if(coeff(3) < lb(3))
        coeff(3) = lb(3);
    end
    if(coeff(1) > ub(1))
        coeff(1) = ub(1);
    end
    if(coeff(2) > ub(2))
        coeff(2) = ub(2);
    end
    if(coeff(3) > ub(3))
        coeff(3) = ub(3);
    end
    % Break on convergence. 
    if(abs(da(1))<tol && abs(da(2))<tol && abs(da(3))<tol)
%        disp(out);
 %       disp('Method has converged.');
        break
    end
end
end


