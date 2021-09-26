FROM python:3.6
#set environment variables
ENV PYTHONDONTWRITEBYTECODE 1
ENV PYTHONUNBUFFERED 1
ENV PYTHONPATH=/usr/src/app:$PYTHONPATH

# set work directory
WORKDIR /usr/src/app

#install packages
RUN apt-get update && apt-get -y install cron vim python-dev default-libmysqlclient-dev python3-dev netcat

# copy source
COPY . /usr/src/app

# install requirements
RUN pip install -r requirements.txt

# run entrypoint.sh
# ENTRYPOINT ["/usr/src/app/init.sh"]
