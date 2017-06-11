var gulp = require('gulp');
var useref = require('gulp-useref');
var gulpIf = require('gulp-if');
var fileinc = require('gulp-file-include');
var serve = require('gulp-serve');
var cssnano = require('gulp-cssnano');
var imagemin = require('gulp-imagemin');
var fs = require('fs-extra');
var klaw = require('klaw');
var path = require('path');
var runSequence = require('run-sequence');
var browserSync = require('browser-sync').create();
var htmlmin = require('gulp-htmlmin');
var gzip = require('gulp-gzip');
var brotli = require('gulp-brotli');
var gzipStatic = require('connect-gzip-static');
var argv = require('yargs').argv;

// Build Dependencies
var browserify = require('browserify');
var vinylBuffer = require('vinyl-buffer');
var gutil = require('gulp-util');
var vinylSourceStream = require('vinyl-source-stream');
var uglify = require('gulp-uglify');
var sourcemaps = require('gulp-sourcemaps');
var pump = require('pump');

// Development Dependencies
var jshint = require('gulp-jshint');

var compileUrl = argv.compileUrl || '//chibitronics.com/compile';

var configHandler =         /* Add a live url, /config.json, that returns our current configuration. */
    function (req, res, next) {
        if (req.url !== '/config.json') {
            return next();
        }
        res.setHeader('Content-Type', 'application/json; charset=utf-8');
        res.end(JSON.stringify({
            compileUrl: compileUrl
        }));
    };

// gulp-serve requires the root directory to exist already.
// Put this here to make sure they can start up.
gulp.task('serve', serve({
    root: 'build',
    middlewares: [configHandler]
}));

// This will fail if there is no "build" directory.
// That's fine, because it's impossible to serve without
// a build directory present.
try {
    gulp.task('serve-prod', serve({
        root: 'build',
        port: 80,
        hostname: "0.0.0.0",
        middlewares: [
            configHandler,
            gzipStatic(__dirname + '/build')
        ]
    }));
} catch (e) { };

gulp.task('build-examples', function() {
    var currentDirectory = '';
    var fileList = [];
    return klaw('examples-ltc')
        .on('data', function(item) {
            if (path.extname(item.path) === '.ino') {
                paths = item.path.split(path.sep);
                currentDirectory = paths[paths.length - 3];
                if (fileList[currentDirectory] === undefined) {
                    fileList[currentDirectory] = [];
                }
                var fileName = paths[paths.length - 1];
                var baseName = path.basename(fileName, '.ino');
                fileList[currentDirectory].push(baseName);
            }
        })
        .on('end', function () {
            var examplesFile = '          <div id="examples_list" class="ExamplesList maintab">\n'
                             + '              <ol class="ExampleOrderedList">\n';
            for (var categoryDir in fileList) {
                var categoryName = categoryDir.split('.');
                categoryName.shift();
                categoryName = categoryName.join('.');

                examplesFile += '                 <li class="ExampleCategory">' + categoryName + '</li>\n';
                examplesFile += '                 <li class="ExampleCategoryContents">\n';
                examplesFile += '                     <ul>\n';
                fileList[categoryDir].forEach(function(exampleName) {
                    examplesFile += '                         <li class="ExampleItem"><a href="examples-ltc/' + categoryDir + '/' + exampleName + '/' + exampleName + '.ino">' + exampleName + '</a></li>\n';
                });
                examplesFile += '                     </ul>\n';
                examplesFile += '                 </li>\n';
            }
            examplesFile += '              </ol>\n'
                          + '          </div>\n';
            fs.outputFileSync('src/examples.html', examplesFile);
        });
});

gulp.task('build-html', function () {
    return gulp.src('src/*.html') /* Load all HTML files */
        .pipe(useref()) /* Combine files into one */
        .pipe(gulpIf('*.css', cssnano())) /* minify css files */
        .pipe(gulpIf('*.html', htmlmin({ collapseWhitespace: false, minifyJS: false, minifyCSS: false }))) /* also minify html */
        .pipe(gulp.dest('build')) /* Write out to 'html' output directory */
});

gulp.task('compress-gz', function () {
    return gulp.src(['build/**/*.html', 'build/**/*.css', 'build/**/*.js', 'build/**/*.map'])
        .pipe(gzip())
        .pipe(gulp.dest('build'))
});

gulp.task('compress-br', function () {
    return gulp.src(['build/**/*.html', 'build/**/*.css', 'build/**/*.js', 'build/**/*.map'])
        .pipe(brotli.compress({
            quality: 11,
            skipLarger: true
        }))
        .pipe(gulp.dest('build'))
});

gulp.task('compress-br-examples', function () {
    return gulp.src('build/examples-ltc/**')
        .pipe(brotli.compress({
            quality: 11,
            skipLarger: true
        }))
        .pipe(gulp.dest('build/examples-ltc'))
});

gulp.task('compress-gz-examples', function () {
    return gulp.src('build/examples-ltc/**')
        .pipe(gzip())
        .pipe(gulp.dest('build/examples-ltc'))
});

gulp.task('lint-src', function () {
    return gulp.src('./src/**/*.js')
        .pipe(jshint())
        .pipe(jshint.reporter('default'))
});

gulp.task('build-scripts', function (cb) {
    // Single entry point to browserify
    var b = browserify({
        debug: true,
        insertGlobals: true,
        entries: 'index.js',
        basedir: 'src/js'
    });

    pump([
        b.bundle(),
        vinylSourceStream('index.js'),
        vinylBuffer(),
        gulp.dest('./build/js')
    ],
        cb
    );

});

gulp.task('build-scripts-minimal', function (cb) {
    // Single entry point to browserify
    var b = browserify({
        debug: false,
        insertGlobals: true,
        entries: 'index.js',
        basedir: 'src/js'
    });

    pump([
        b.bundle(),
        vinylSourceStream('index.js'),
        vinylBuffer(),
        sourcemaps.init({ loadMaps: true }),
        // Add gulp plugins to the pipeline here.
        uglify(),
        sourcemaps.write('./'),
        gulp.dest('./build/js')
    ],
        cb
    );
});

gulp.task('build-lame', function () {
    return gulp.src('node_modules/lamejs/lame.min.js')
        .pipe(gulp.dest('build/js/'))
});

gulp.task('build-jscolor', function () {
    return gulp.src('colorjs/jscolor.js')
        .pipe(gulp.dest('build/js/'))
});

gulp.task('copy-examples', function () {
    return gulp.src('examples-ltc/**/*')
        .pipe(gulp.dest('build/examples-ltc'))
});

gulp.task('copy-images', function () {
    return gulp.src('src/images/**/*.{png,gif,jpg,svg}')
        .pipe(imagemin())
        .pipe(gulp.dest('build/images'))
});

gulp.task('clean:build', function () {
    fs.ensureDirSync('build');
    fs.removeSync('build');
    return fs.ensureDirSync('build');
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
	'lint-src', 'build-scripts', 'build-lame', 'build-jscolor',
        'copy-images',
        'copy-examples',
    ],
        callback
    );
});

gulp.task('default', function (callback) {
    runSequence('clean:build', 'build', 'build-scripts-minimal',
        [
            'compress-gz',
            'compress-br',
            'compress-gz-examples',
            'compress-br-examples'
        ],
        callback
    );
});

gulp.task('watch', ['browserSync'], function (callback) {
    gulp.watch('src/*.html', ['build-html', browserSync.reload]);
    gulp.watch('src/**/*.css', ['build-html', browserSync.reload]);
    gulp.watch('src/js/**/*.js', ['build-scripts', browserSync.reload]);
    runSequence('clean:build', 'build', callback);
});
