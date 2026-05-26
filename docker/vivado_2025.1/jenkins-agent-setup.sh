#!/bin/bash
# jenkins-agent-setup.sh

/usr/sbin/sshd -D &

source /opt/Xilinx/2025.1/Vivado/settings64.sh

if [ -n "$JENKINS_SECRET" ] && [ -n "$JENKINS_URL" ]; then
    exec java -jar /usr/share/jenkins/agent.jar \
        -url "$JENKINS_URL" \
        -secret "$JENKINS_SECRET" \
        -name "$JENKINS_AGENT_NAME" \
        -workDir "/home/jenkins/agent"
else
    exec /bin/bash
fi
