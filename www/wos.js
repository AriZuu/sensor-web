(function( $ ){

  $.fn.wosTimer = function(delay, fn) {
  
    function doTimer(elem) 
    {
      var timerData = elem.data('wosTimer');

      var d = {};
      d.elem = elem;

      timerData.timer = window.setTimeout(function() {

        timerData.timer = null;
        if (timerData.fn.call(d.elem))
          doTimer(d.elem);
          
      }, timerData.delay);
    }

    return this.each(function() {

      var timerData = $(this).data('wosTimer');

      if (!timerData) {

         timerData = {};
        $(this).data('wosTimer', timerData);
      }

      if (timerData.timer) {

        window.clearTimeout(timerData.timer);
        timerData.timer = null;
      }

      if (delay != 0) {

        timerData.delay = delay;
        if (fn)
          timerData.fn = fn;

        doTimer($(this));
      } 

    });
  };

  $.fn.wosJSON = function(delay, url, fn) {
  
    function doAjax()
    {
      var ajaxData = this.data('wosAjax');
         
      ajaxData.busy = true;

      $.ajax({

        url: ajaxData.url,
        cache: false,
        context : this,
        dataType: 'json',
        success: function (data) {

          if (!ajaxData.fn.call(this, data))
            ajaxData.delay = 0;

          if (ajaxData.delay != 0)
            this.wosTimer(ajaxData.delay, doAjax);
        },
        error: function () {

          if (!ajaxData.fn.call(this, null))
            ajaxData.delay = 0;

          if (ajaxData.delay != 0)
            this.wosTimer(60000, doAjax);
        },
        complete: function() {

          ajaxData.busy = false;
        }

      });

      return false;
    }

    return this.each(function() {

      var ajaxData = $(this).data('wosAjax');
      if (!ajaxData) {

        ajaxData = {};
        ajaxData.busy  = false;
        $(this).data('wosAjax', ajaxData);
      }

      if (delay != 0) {

        ajaxData.delay = delay;

        if (url)
          ajaxData.url = url;

        if (fn)
          ajaxData.fn = fn;

        if (!ajaxData.busy)
          doAjax.call($(this));
      } 
      else if (!ajaxData.busy)
        $(this).wosTimer(0);

    });
  };
})( jQuery );

