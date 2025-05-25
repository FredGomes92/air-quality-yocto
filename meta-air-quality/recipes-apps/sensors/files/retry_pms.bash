#!/bin/sh
MAX_RETRIES=5
DELAY=10
COUNT=0

while [ $COUNT -lt $MAX_RETRIES ]; do
    sensor-pms /dev/ttyAMA0
    STATUS=$?
    echo "Sensor PMS5003 exited with status $STATUS"
    if [ $STATUS -eq 0 ]; then
        exit 0
    elif [ $STATUS -eq 1 ]; then
        COUNT=$((COUNT + 1))
        sleep $DELAY
    else
        exit $STATUS
    fi
done

exit 1