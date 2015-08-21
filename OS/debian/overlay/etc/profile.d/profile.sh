export PS1="\e[0;31mred\e[1mpitaya\e[0;31m>\e[m "
export PATH=/sbin:/usr/sbin:/bin:/usr/bin

export USER=`id -un`
export LOGNAME=$USER
export HOSTNAME=`/bin/hostname`
export HISTSIZE=1000
export HISTFILESIZE=1000
export PAGER='/usr/bin/less '
export EDITOR='/usr/bin/nano'
export INPUTRC=/etc/inputrc

export LS_OPTIONS='--color=auto'
eval "`dircolors`"
alias ls='ls $LS_OPTIONS'
