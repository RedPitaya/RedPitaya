function s2=fft_interp(s1,nn)

% FFT based interpolation algorithm
% Borut Baricevic
%
% s2=fft_interp(s1,nn)
%
% s1: input signal
% nn: samples to be added to the signal as interpolation

ll=length(s1);

% Fourier domain
ff1=fft(s1);

%Last spectrum sample
ll_last=floor(ll/2)-1; 

%Insert nn zeros into spectrum for frequencies beyond original fs/2, preserve 0 to fs/2 frequency content
tmp=[ff1(1:ll_last),zeros(1,nn), conj(ff1(ll_last:-1:2))];

%Return to time domail and adjust scaling
s2=real(ifft(tmp))*(nn+ll)/ll;

end