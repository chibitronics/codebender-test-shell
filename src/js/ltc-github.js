'use strict';
var LTCGitHub = function(params) {
    if (!params) {
        params = {};
    }

    // Settable parameters
    this.gatewayUrl = params.gatewayUrl || 'http://localhost:9999/authenticate/';
    this.gitHubUrl = params.gitHubUrl || 'https://api.github.com';
    this.gitHubTokenUrl = params.gitHubTokenUrl;
    this.gitHubRepoName = params.gitHubRepoName || 'Love-to-Code-storage';

    // A synchronized table of files on the server, including their SHA sums.
    this.files = {};

    // Listen for messages coming from github windows
    var self = this;
    window.addEventListener('message', function(event) {
        if (event.data.githubCode === undefined) {
            return;
        }
        self.loginWithCode(event.data.githubCode);
    });
};

LTCGitHub.prototype = {
    getToken: function() {
        return localStorage.getItem('gitHubToken');
    },

    getLogin: function() {
        return localStorage.getItem('gitHubLogin');
    },

    setToken: function(t) {
        localStorage.setItem('gitHubToken', t);
    },

    setLogin: function(l) {
        localStorage.setItem('gitHubLogin', l);
    },

    clear: function() {
        localStorage.removeItem('gitHubToken');
        localStorage.removeItem('gitHubLogin');
    },

    refreshRepo: function(cb) {
        this.doApiCall('GET', '/repos/' + this.getLogin() + '/' + this.gitHubRepoName + '/contents/', undefined, function(txt, status) {
            // If the repo doesn't exist, create a new one.
            if (status === 404) {
                console.log('Chibitronics repo doesn\'t exist.  Creating...');
                this.doApiCall('POST', '/user/repos', {
                    'name': this.gitHubRepoName,
                    'description': 'Online storage for Love-to-Code sketches',
                    'homepage': 'https://ltc.chibitronics.com',
                    'private': false,
                    'has_issues': false,
                    'has_projects': false,
                    'has_wiki': false
                }, function(txt, status) {
                    console.log('Got result from checkForChibitronicsRepo():');
                    console.log(txt)
                    console.log('Status: ' + status);
                    if (status >= 200 && status <= 299) {
                        if (cb !== undefined) {
                            cb();
                        }
                    }
                });
            } else {
                console.log('Repo exists already');

                // Parse the list of returned files into a local cache of arrays
                var self = this;
                JSON.parse(txt).forEach(function(item) {
                    if (item.type !== 'file') {
                        return;
                    }

                    self.files[item.name] = {
                        'sha': item.sha
                    };
                });

                // Call the callback, now that we have a populated list of files.
                if (cb !== undefined) {
                    cb();
                }
            }
        });
    },

    loginWithCode: function(code) {

        // Once we get the token, bounce it to the "gatekeeper" gateway
        // to mark the token as "live", and turn the "code" into a "token".
        this.doApiCall('GET', this.gatewayUrl + code, undefined, function(txt) {
            var data = JSON.parse(txt);
            this.setToken(data.token);

            // Now that we're authenticated, figure out our username.
            this.doApiCall('GET', '/user', undefined, function(txt) {
                var d2 = JSON.parse(txt);
                this.setLogin(d2.login);

                if (this.loginCallback !== undefined) {
                    this.loginCallback();
                    this.loginCallback = undefined;
                }
            });
        });
    },

    doApiCall: function(method, path, data, cb) {
        var request = new window.XMLHttpRequest();
        var self = this;
        request.onload = function() {
            cb.call(self, request.responseText, request.status);
        };

        // Prefix the URL with the github API path, if it's relative.
        var url = path;
        if (!path.startsWith("http://") && !path.startsWith("https://")) {
            url = this.gitHubUrl + path;
        }

        request.open(method, url, true);

        // If we're already authenticated, add the token.  This is omitted during
        // the login process.
        if (this.getToken() !== null) {
            request.setRequestHeader('Authorization', 'token ' + this.getToken());
        }

        if (typeof data === 'object') {
            data = JSON.stringify(data);
        }
        request.send(data);
    },

    // We're considered logged out if we have no token or no username.
    loggedIn: function() {
        if ((this.getToken() === null) || (this.getLogin() === null)) {
            return false;
        }
        return true;
    },

    login: function(cb) {
        if (!this.loggedIn()) {
            window.open(this.gitHubTokenUrl);
            this.loginCallback = cb;
        } else {
            cb(1);
        }
    },

    writeFile: function(fileName, contents, cb) {
        this.doApiCall('PUT', '/repos/' + this.getLogin() + '/' + this.gitHubRepoName + '/contents/' + fileName, {
            'path': fileName,
            'message': 'Saving file',
            'content': btoa(contents),
        }, function(txt, status) {
            if (cb !== undefined) {
                cb(txt, status);
            }
        });
    }
};

// AMD exports
if (typeof module !== 'undefined' && module.exports) {
    module.exports = LTCGitHub;
}