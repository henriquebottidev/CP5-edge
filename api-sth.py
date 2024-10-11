import dash
from dash import dcc, html
from dash.dependencies import Input, Output, State
import plotly.graph_objs as go
import requests
from datetime import datetime
import pytz

# Constants for IP and port
IP_ADDRESS = "18.209.40.125"
PORT_STH = 8666
DASH_HOST = "0.0.0.0" # Set this to "0.0.0.0" to allow

#Function to get luminosity data
def get_luminosity_data(lastN):
    url = f"http://{IP_ADDRESS}:{PORT_STH}/STH/v1/contextEntities/type/Lamp/id/urn:ngsi-ld:Lamp:003/att
ributes/luminosity?lastN={lastN}" 
    headers = {
        'fiware-service': 'smart',
'fiware-servicepath': '/'
    }
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        data = response.json()
        try:
            values = data['contextResponses'][0]['contextElement']['attributes'][0]['values'] 
            return values
        except KeyError as e:
            print(f"Key error: {e}")
            return []
    else:
        print(f"Error accessing {url}: {response.status_code}")
        return []

# Function to get temperature data
def get_temperature_data(lastN):
    url = f"http://{IP_ADDRESS}:{PORT_STH}/STH/v1/contextEntities/type/Lamp/id/urn:ngsi-ld:Lamp:003/att
ributes/temperature?lastN={lastN}"
    headers = {
        'fiware-service': 'smart',
        'fiware-servicepath': '/'
    }
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        data = response.json()
        try:
            values = data['contextResponses'][0]['contextElement']['attributes'][0]['values'] 
            return values
        except KeyError as e:
            print(f"Key error: {e}")
            return []
    else:
        print(f"Error accessing {url}: {response.status_code}")
        return []

# Function to get humidity data
def get_humidity_data(lastN):
    url = f"http://{IP_ADDRESS}:{PORT_STH}/STH/v1/contextEntities/type/Lamp/id/urn:ngsi-ld:Lamp:003/att
ributes/humidity?lastN={lastN}" 
    headers = {
        'fiware-service': 'smart',
        'fiware-servicepath': '/'
    }
    response = requests.get(url, headers=headers)
    if response.status_code == 200:
        data = response.json()
        try:
            values = data['contextResponses'][0]['contextElement']['attributes'][0]['values'] 
            return values
        except KeyError as e:
            print(f"Key error: {e}")
            return []
    else:
        print(f"Error accessing {url}: {response.status_code}")
        return []

# Function to convert UTC timestamps to Lisbon time
def convert_to_lisbon_time(timestamps):
    utc = pytz.utc
    lisbon = pytz.timezone('Europe/Lisbon')
    converted_timestamps = []
    for timestamp in timestamps:
        try:
            timestamp = timestamp.replace('T', ' ').replace('Z', '')
            converted_time = utc.localize(datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S.%f')).astimezone(lisbon)
        except ValueError:
            # Handle case where milliseconds are not present
            converted_time = utc.localize(datetime.strptime(timestamp, '%Y-%m-%d %H:%M:%S')).astimezone(lisbon)
        converted_timestamps.append(converted_time)
    return converted_timestamps

# Set lastN value
lastN = 10 # Get 10 most recent points at each interval

app = dash.Dash(__name__)

app.layout = html.Div([
    html.H1('Sensor Data Viewer'),

    # Graph for Luminosity
    dcc.Graph(id='luminosity-graph'),
    dcc.Store(id='luminosity-data-store', data={'timestamps': [], 'luminosity_values': []}),

    # Graph for Temperature
    dcc.Graph(id='temperature-graph'),
    dcc.Store(id='temperature-data-store', data={'timestamps': [], 'temperature_values': []}),

    # Graph for Humidity
    dcc.Graph(id='humidity-graph'),
    dcc.Store(id='humidity-data-store', data={'timestamps': [], 'humidity_values': []}),

    dcc.Interval(
        id='interval-component',
        interval=10*1000, # in milliseconds (10 seconds)
        n_intervals=0
    )
])

@app.callback(
     [Output('luminosity-data-store', 'data'),
      Output('temperature-data-store', 'data'),
      Output('humidity-data-store', 'data')],
    Input('interval-component', 'n_intervals'),
     [State('luminosity-data-store', 'data'),
      State('temperature-data-store', 'data'),
      State('humidity-data-store', 'data')]
)

def update_data_store(n, stored_luminosity, stored_temperature, stored_humidity):
    # Get luminosity, temperature, and humidity data
    data_luminosity = get_luminosity_data(lastN)
    data_temperature = get_temperature_data(lastN)
    data_humidity = get_humidity_data(lastN)

    if data_luminosity:
        luminosity_values = [float(entry['attrValue']) for entry in data_luminosity ]
        timestamps = [entry['recvTime'] for entry in data_luminosity]

        timestamps = convert_to_lisbon_time(timestamps)

        stored_luminosity['timestamps'].extend(timestamps)
        stored_luminosity['luminosity_values'].extend(luminosity_values)

    if data_temperature:
        temperature_values = [float(entry['attrValue']) for entry in data_temperature]
        stored_temperature['temperature_values'] = temperature_values

    if data_humidity:
        humidity_values = [float(entry['attrValue']) for entry in data_humidity]
        stored_humidity['humidity_values'] = humidity_values

    return stored_luminosity, stored_temperature, stored_humidity

@app.callback(
    [Output('luminosity-graph', 'figure'),
     Output('temperature-graph', 'figure'),
     Output('humidity-graph', 'figure')],
    [Input('luminosity-data-store', 'data'),
     Input('temperature-data-store', 'data'),
     Input('humidity-data-store', 'data')]
)

def update_graphs(luminosity_data, temperature_data, humidity_data):
    # Luminosity graph
    luminosity_trace = go.Scatter(
        x=luminosity_data['timestamps'],
        y=luminosity_data['luminosity_values'],
        mode='lines+markers',
        name='Luminosity'
    )
    luminosity_layout = go.Layout(title='Luminosity over Time')

    # Temperature graph
    temperature_trace = go.Scatter(
        y=temperature_data['temperature_values'],
        mode='lines+markers',
        name='Temperature',
        line = dict(color='red')
    )
    temperature_layout = go.Layout(title='Temperature over Time')

    # Humidity graph
    humidity_trace = go.Scatter(
        y=humidity_data['humidity_values'],
        mode='lines+markers',
        name='Humidity',
        line = dict(color='green')
    )
    humidity_layout = go.Layout(title='Humidity over Time')


    return {
        'data': [luminosity_trace],
        'layout': luminosity_layout
    }, {
        'data': [temperature_trace],
        'layout': temperature_layout
    }, {
        'data': [humidity_trace],
        'layout': humidity_layout
    }

if __name__ == '__main__':
    app.run_server(debug=True, host=DASH_HOST, port=8050)