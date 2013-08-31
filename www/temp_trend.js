function setData(data) {

  var i;
  
  if (data != null) {

    for (i = 0; i < data.length; i++)
      data[i].timeStamp = new Date().getTime() + data[i].timeStamp * 1000 * 3600;

    chart.setData(data);
  }
}

var chart = new Morris.Line({
  element: 'chartdiv',
  data: [],
  xkey: 'timeStamp',
  ykeys: ['temp'],
  labels: ['Value']
});

function tempActive() {

  return ($('#tabs').tabs('option', 'selected') == 1);
}

$('#tabs').on('tabsshow', function (event, ui) {

  $('#chartdiv').wosJSON(tempActive() ? 300000 : 0);
});

$('#chartdiv').wosJSON(300000, "temp_trend_data.cgi", function(data) {

  if (!tempActive())
     return false;

  setData(data);
  return true;
});

