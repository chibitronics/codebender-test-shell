var gulp = require('gulp');
var useref = require('gulp-useref');
var uglify = require('gulp-uglify');
var gulpIf = require('gulp-if');
var cssnano = require('gulp-cssnano');
var imagemin = require('gulp-imagemin');
var del = require('del');
var runSequence = require('run-sequence');
var browserSync = require('browser-sync').create();
var htmlmin = require('gulp-htmlmin');

gulp.task('hello', function() {
    console.log("Hello, world!");
});

gulp.task('useref', function() {
    return gulp.src('app/*.html') /* Load all HTML files */
        .pipe(useref()) /* Combine files into one */
        //.pipe(gulpIf('*.js', uglify())) /* minify javascript files */
        //.pipe(gulpIf('*.css', cssnano())) /* minify css files */
        .pipe(gulpIf('*.html', htmlmin({ collapseWhitespace: false, minifyJS: false, minifyCSS: false }))) /* also minify html */
        .pipe(gulp.dest('html')) /* Write out to 'html' output directory */
});

gulp.task('cpjson', function() {
    return gulp.src('app/*.json')
        .pipe(gulp.dest('html'))
});

gulp.task('cpexamples', function() {
    return gulp.src('app/examples/**/*')
        .pipe(gulp.dest('html/examples'))
});

gulp.task('cpimages', function() {
    return gulp.src('app/images/**/*.{png,gif,jpg,svg}')
        .pipe(imagemin())
        .pipe(gulp.dest('html/images'))
});

gulp.task('clean:html', function() {
    return del.sync('html');
});

gulp.task('cache:clear', function(callback) {
    return cache.clearAll(callback);
});

gulp.task('browserSync', function() {
    browserSync.init({
        server: {
            baseDir: 'app'
        }
    })
});

gulp.task('watch', ['browserSync'], function() {
    gulp.watch('app/*.html', browserSync.reload);
    gulp.watch('app/js/**/*.js', browserSync.reload);
});

gulp.task('build', function(callback) {
    runSequence('clean:html', ['useref', 'cpimages', 'cpjson', 'cpexamples'],
        callback
    );
});