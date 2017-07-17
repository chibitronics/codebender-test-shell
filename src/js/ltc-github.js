'use strict';
var LTCGitHub = function(params) {
    if (!params) {
        params = {};
    }

    this.gatewayUrl = params.gatewayUrl || 'http://localhost:9999/authenticate/';
    this.gitHubTokenUrl = params.gitHubTokenUrl;
    this.gitHubRepoName = params.gitHubRepoName || 'Love-to-Code-storage';

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

    ensureRepo: function() {
        var self = this;
        var request = new window.XMLHttpRequest();
        request.onload = function() {
            // If the repo doesn't exist, create a new one.
            if (request.status === 404) {
                console.log('Chibitronics repo doesn\'t exist.  Creating...');
                // Ensure that the Chibitronics repo exists
                var createRepoData = {
                    'name': self.gitHubRepoName,
                    'description': 'Online storage for Love-to-Code sketches',
                    'homepage': 'https://ltc.chibitronics.com',
                    'private': false,
                    'has_issues': false,
                    'has_projects': false,
                    'has_wiki': false
                };

                var req2 = new window.XMLHttpRequest();
                req2.onload = function() {
                    console.log('Got result from checkForChibitronicsRepo():');
                    console.log(req2.responseText);
                    console.log(req2);
                };
                req2.open('POST', 'https://api.github.com/user/repos', true);
                req2.setRequestHeader('Authorization', 'token ' + self.getToken());
                req2.send(JSON.stringify(createRepoData));
            } else {
                console.log('Repo exists already');
            }
        };
        request.open('GET', 'https://api.github.com/repos/' + self.getLogin() + '/' + self.gitHubRepoName, true);
        request.setRequestHeader('Authorization', 'token ' + self.getToken());
        request.send();
    },

    loginWithCode: function(code) {

        // Once we get the token, bounce it to the "gatekeeper" gateway
        // to mark the token as "live", and turn the "code" into a "token".
        this.doApiCall('GET', this.gatewayUrl + code, undefined, function(txt) {
            var data = JSON.parse(txt);
            this.setToken(data.token);

            // Now that we're authenticated, figure out our username.
            this.doApiCall('GET', 'https://api.github.com/user', undefined, function(txt) {
                var d2 = JSON.parse(txt);
                this.setLogin(d2.login);

                if (this.loginCallback !== undefined) {
                    this.loginCallback();
                    this.loginCallback = undefined;
                }
            });
        });
    },

    doApiCall: function(method, url, data, cb) {
        var request = new window.XMLHttpRequest();
        var self = this;
        request.onload = function() {
            cb.call(self, request.responseText, request.status);
        };
        request.open(method, url, true);
        if (self.getToken() !== null) {
            request.setRequestHeader('Authorization', 'token ' + this.getToken());
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
    }
};

// AMD exports
if (typeof module !== 'undefined' && module.exports) {
    module.exports = LTCGitHub;
}