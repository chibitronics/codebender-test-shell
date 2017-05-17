FROM node:7.8.0
#FROM armhf/node:7.8.0 # arch=armhf

COPY . /app
WORKDIR /app

RUN npm install --quiet --global gulp-cli
RUN npm install --quiet

RUN gulp

EXPOSE 80
CMD ./entrypoint.sh
