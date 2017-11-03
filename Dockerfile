FROM node:9.0.0

# use changes to package.json to force Docker not to use the cache
# when we change our application's nodejs dependencies:
COPY package.json /tmp/package.json
RUN cd /tmp && npm install --quiet --global gulp-cli && npm install --quiet
RUN mkdir -p /app && cp -a /tmp/node_modules /app/

WORKDIR /app
COPY . /app

RUN gulp

EXPOSE 80
CMD ./entrypoint.sh
