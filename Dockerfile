FROM node:7.9.0

COPY . /app
WORKDIR /app

RUN npm install --global gulp-cli
RUN npm install

# This is required.  I honestly don't know why.  Otherwise,
# optipng doesn't get created.
#RUN npm uninstall gulp-imagemin
#RUN npm install gulp-imagemin

RUN gulp build

EXPOSE 80
CMD ["gulp", "serve-prod"]