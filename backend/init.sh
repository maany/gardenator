#!/bin/bash

# gunicorn
cp /usr/src/app/config/gunicorn /etc/init.d/gunicorn
cp /usr/src/app/config/gunicorn.socket /etc/systemd/system

# wait for postgres
if [[ "$MODE" = "dev" ]]
then
    echo "Waiting for postgres..."

    while ! nc -z db 5432; do
      sleep 0.1
    done

    echo "PostgreSQL started"
fi

#django
export PYTHONPATH=/code:${PYTHONPATH}
python manage.py makemigrations
python manage.py migrate
echo "yes" | python manage.py collectstatic
python manage.py ensure_adminuser --username='admin' \
    --email=admin@example.com \
    --password=${ADMIN_PASSWORD}
#echo "from django.contrib.auth.models import User; User.objects.create_superuser('admin', 'admin@example.com', '${ADMIN_PASSWORD}')" | python manage.py shell


if [[ $MODE = "prod" ]]
then
    gunicorn sq_api_gateway.wsgi:application --bind 0.0.0.0:8000
else
    python manage.py runserver 0.0.0.0:8000
fi
