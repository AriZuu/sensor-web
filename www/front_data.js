$(function () {

  $.ajaxSetup({

    cache: true
  });

  $("#tabs").tabs({

    cache: true,
    ajaxOptions: {

      error: function (xhr, status, index, anchor) {
	$(anchor.hash).html("Couldn't load this tab.");
      }
    }
  });

  function frontActive() {

    return ($('#tabs').tabs('option', 'selected') == 0);
  }

  $('#tabs').on('tabsshow', function (event, ui) {

     $('#tabs-1').wosJSON(frontActive() ? 10000 : 0);
  });

  $('#tabs-1').wosJSON(10000, "front_data.cgi", function(data) {

    if (!frontActive())
       return false;

    if (data) {

      var rowCount = this.find('.dataRow').size();

      while (rowCount < data.length) {

        var newRow = this.find('.template').clone();
        newRow.attr("id", 'row' + rowCount);
        newRow.attr("class", 'dataRow');
        this.find('tbody').append(newRow);
        ++rowCount;
      }

      var i;

      i = 0;
      while (i < rowCount) {

        if (i < data.length) {

          for (var id in data[i]) 
            if (id == "tempHistory") {

              this.find('#row' + i +' #tempHistory').sparkline(data[i].tempHistory, {
                type: 'line'
              });
            }
            else
               this.find('#row' + i +' #' + id).text(data[i][id]);
        }
        else
          this.find('#row' + i +' span').text('--');


        ++i;
      }
    }
    else {

      this.find('span').text('--');
    }

    return true;
  });
});
