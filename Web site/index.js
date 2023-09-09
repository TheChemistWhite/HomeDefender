const config = {
    api_endpoint: 'https://a1o549s933v6t7-ats.iot.us-east-1.amazonaws.com/homeDefender_data'
  };
  var values_x = [];
  var coluomns_colors = []; 
  var colors = ["blue", "red", "black"]
  var current_color = 0;
  var values_y = [];
  var ch1;
  var ch1Stats; 
  var values_y_stats = [];
  
  function displayDataOnChart(data_input) {
    console.log(data_input);
    const parsed_data = JSON.parse(data_input);
  
    values_x = [];
    values_y = [];
    input = [];
  
    parsed_data.sort((a, b) => {
      return new Date(a.sample_time) - new Date(b.sample_time); 
    });
  
    for (let i = 0; i < parsed_data.length; i++) {
      coluomns_colors.push(colors[1]);
      const sampleTime = new Date(parsed_data[i]["sample_time"]);
      label = sampleTime.toLocaleString(); 
  
      values_x.push(label); 
  
      values_y.push(parsed_data[i].device_data.flame);
  
      input.push([parsed_data[i].sample_time, parsed_data[i].device_data.flame]);
    }
  
    // Update the chart with the new data
    ch1.data.labels = values_x;
    ch1.data.datasets[0].data = values_y;
    ch1.update();
  
    calculateStatistics();
  }
  
  function calculateStatistics() {
    const lastHourData = input; 
  
    const pirStats = calculateAggregatedStats(lastHourData);
  
    displayStatsOnChart(pirStats);
  }
  
  
  function displayStatsOnChart(pirStats) {
    const statsCanvas1 = document.getElementById("Chart1Stats");
  
    ch1Stats = new Chart(statsCanvas1, {
      type: "bar",
      data: {
        labels: [],
        datasets: [{
          backgroundColor: colors[1],
          data: [
            pirStats.toFixed(2)
          ]
        }]
      },
      options: {
        legend: { display: false },
        title: { display: true },
        scales: {
          yAxes: [{
            ticks: { beginAtZero: true }
          }]
        }
      }
    });
  }
  
  function callAPI(){
    var headers = new Headers();
    var requestOptions = {
        method: 'GET',
        headers: headers,
    };
    
    fetch(config.api_endpoint, requestOptions)
    .then(response => response.text()).then(result => displayDataOnChart(result))
  }
  
  function init() {
    ch1 = createPlot("Chart1", values_y);
    callAPI();
    setInterval(callAPI, 60000); // Refresh the data every minute
  } 
  
  init();