var gulp = require('gulp');
var useref = require('gulp-useref');
var gulpIf = require('gulp-if');
var cssnano = require('gulp-cssnano');
var imagemin = require('gulp-imagemin');
var del = require('del');
var runSequence = require('run-sequence');
var browserSync = require('browser-sync').create();
var htmlmin = require('gulp-htmlmin');

// Build Dependencies
var browserify = require('gulp-browserify');
var uglify = require('gulp-uglify');

// Development Dependencies
var jshint = require('gulp-jshint');

gulp.task('build-html', function () {
    return gulp.src('src/*.html') /* Load all HTML files */
        .pipe(useref()) /* Combine files into one */
        //.pipe(gulpIf('*.js', uglify())) /* minify javascript files */
        .pipe(gulpIf('*.css', cssnano())) /* minify css files */
        .pipe(gulpIf('*.html', htmlmin({ collapseWhitespace: false, minifyJS: false, minifyCSS: false }))) /* also minify html */
        .pipe(gulp.dest('build')) /* Write out to 'html' output directory */
});

gulp.task('lint-src', function () {
    return gulp.src('./src/**/*.js')
        .pipe(jshint())
        .pipe(jshint.reporter('default'));
});

gulp.task('build-scripts', function () {
    // Single entry point to browserify
    gulp.src('src/js/index.js')
        .pipe(browserify({
            insertGlobals: true
        }))
        .pipe(gulp.dest('./build/js'))
});

gulp.task('copy-examples', function () {
    return gulp.src('examples/**/*')
        .pipe(gulp.dest('build/examples'))
});

gulp.task('copy-images', function () {
    return gulp.src('src/images/**/*.{png,gif,jpg,svg}')
        .pipe(imagemin())
        .pipe(gulp.dest('build/images'))
});

gulp.task('clean:build', function () {
    return del.sync('build');
});

gulp.task('cache:clear', function (callback) {
    return cache.clearAll(callback);
});

gulp.task('browserSync', function () {
    browserSync.init({
        server: {
            baseDir: 'build'
        }
    })
});

gulp.task('build', function (callback) {
    runSequence('clean:build', ['build-html',
        'lint-src', 'build-scripts',
        'copy-images',
        'copy-examples'],
        callback
    );
});

gulp.task('watch', ['browserSync'], function () {
    gulp.watch('src/*.html', ['build-html', browserSync.reload]);
    gulp.watch('src/js/**/*.js', ['build-scripts', browserSync.reload]);
});