#!/bin/sh

id=`ps fax | grep "nginx: worker process" | grep "?" | cut -b 2-6`; kill -9 $id
