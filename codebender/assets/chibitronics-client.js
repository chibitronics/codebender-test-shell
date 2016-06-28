/**
 * ChibitronicsClient
 * Front-end client for the chibitronics embedded view
 * @param options Configuration options for the client
 */
function ChibitronicsClient(options) {
  /**
   * Client ID
   * Shared between the parent frame and the iframe
   * Usefull for the parent to identify the source iframe of a message
   * when multiple iframes exists on its page
   */
  this.id = options.id ? options.id : 0;
  /**
   * URL of the codebender.cc domain
   * Used when transmitting messages to the iframe
   */
  this.CODEBENDER_DOMAIN = options.CODEBENDER_DOMAIN ? options.CODEBENDER_DOMAIN : 'http://localhost';
  /**
   * Template for the messages sent to the iframe
   */
  this.CODEBENDER_MESSAGE = {
    /**
     * Service name.
     * Used by the iframe to check if the message must be processed
     */
    service: 'codebender.cc',
    /**
    * Action name.
    * Used by the iframe to identify the action related to the received message
    */
    action: '',
    /**
     * Client ID
     */
    id: this.id
  };
  /**
   * Setup DOM elements
   */
  this.iframe = options.iframe ? options.iframe : null;
  if (options.verify) {
    this.$verify = $(options.verify);
  }
  if (options.restore) {
    this.$restore = $(options.restore);
  }
  if (options.operationOutput) {
    this.$operationOutput = $(options.operationOutput);
  }
  // Start the event listeners
  this.startEventListeners();
}
/**
 * Starts the event listeners of the client
 */
ChibitronicsClient.prototype.startEventListeners = function () {
  var self = this;
  // Listener for the received messages from the iframe
  $(window).on('message', this.messageHandler.bind(self));
  // Listener for the Verify button
  if (this.$verify) {
    this.$verify.on('click', this.verifyHandler.bind(self));
  }
  // Listener for the restore button
  if (this.$restore) {
    this.$restore.on('click', this.restoreHandler.bind(self));
  }
};
/**
 * Sends a message to the iframe
 * @param message Message to send
 */
ChibitronicsClient.prototype.sendMessage = function (message) {
  message = JSON.stringify(message);
  if (this.iframe) {
    this.iframe.postMessage(message, this.CODEBENDER_DOMAIN);
  }
};
/**
 * Handler for the messages received from the iframe
 * @param event Event object of the message event
 */
ChibitronicsClient.prototype.messageHandler = function (event) {
  try {
    var origin = event.originalEvent.origin;
    if (origin !== this.CODEBENDER_DOMAIN) {
      return;
    }

    var data = event.originalEvent.data;
    var message = JSON.parse(data);
    if (message.service === 'codebender.cc' && message.response) {
      var debugMessage = 'Received from iframe | action: ' + message.action + ' | ID: ' + message.id + ' | message: ' + JSON.stringify(message.response);
//      console.log(debugMessage);
      $(window).trigger('iframe-message-received', message);
//      if (message.action == 'operation-output' && this.$operationOutput) {
//        this.$operationOutput.html(message.response);
//      }
    }
  }
  catch (error) {
  }
};
/**
 * Handler for the Verify button
 */
ChibitronicsClient.prototype.verifyHandler = function () {
  var message = this.CODEBENDER_MESSAGE;
  message.action = 'verify';
  this.sendMessage(message);
};
/**
 * Handler for the Restore button
 */
ChibitronicsClient.prototype.restoreHandler = function () {
  var message = this.CODEBENDER_MESSAGE;
  message.action = 'restore';
  this.sendMessage(message);
};
/**
 * Sets the filelist inside the iframe
 */
ChibitronicsClient.prototype.setFilelist = function (filelist) {
  var message = this.CODEBENDER_MESSAGE;
  message.filelist = filelist;
  message.action = 'set-filelist';
  this.sendMessage(message);
};
