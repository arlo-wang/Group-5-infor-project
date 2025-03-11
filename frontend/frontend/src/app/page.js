'use client'; // Make sure this is a client component since it uses hooks

import { React, useState, useEffect } from 'react';
import { GoogleMap, Circle, useLoadScript } from '@react-google-maps/api';

const GeofenceMap = () => {
  const [geofences, setGeofences] = useState([])

  const fetchGeofences = () => {
    fetch(`http://3.8.78.228:8000/api/cart/location/`, {
      method: "GET",
      headers: {
          'Content-Type': 'application/json'
      }}).then((res) => {
      if (res.ok) {
          res.json().then((data) => {
              setGeofences(data);
          })
      } else {
        setGeofences([
          {
            center: { lat: 51.498, lng: -0.176 },
            radius: 10, // meters
          },
          {
            center: { lat: 51.494, lng: -0.184 },
            radius: 200, // meters
          },
          {
            center: { lat: 51.496, lng: -0.174 },
            radius: 10
          }
        ]);
      }
    }).catch(_ => {
      setGeofences([
        {
          center: { lat: 51.498, lng: -0.176 },
          radius: 10, // meters
        },
        {
          center: { lat: 51.494, lng: -0.184 },
          radius: 200, // meters
        },
        {
          center: { lat: 51.496, lng: -0.174 },
          radius: 10
        }
      ]);
    })
  }

  useEffect(() => {
    fetchGeofences();
    console.log(geofences);
    const intervalId = setInterval(fetchGeofences, 5000);
    return () => clearInterval(intervalId);
  }, [])
  // Use useLoadScript hook
  const { isLoaded, loadError } = useLoadScript({
    googleMapsApiKey: process.env.NEXT_PUBLIC_GOOGLE_MAPS_API_KEY || '', 
    libraries: ['geometry'],
  });

  if (loadError) {
    return <div>Error loading maps</div>;
  }
  if (!isLoaded) {
    return <div>Loading maps...</div>;
  }

  // Return the JSX structure
  return (
    <div style={{
      position: 'fixed', // Fixed positioning to cover the entire viewport
      top: 0,
      left: 0,
      right: 0,
      bottom: 0,
      width: '100vw', // Viewport width
      height: '100vh', // Viewport height
      overflow: 'hidden', // Prevent scrollbars if map content overflows (unlikely in this case)
    }}>
      {/* No need for the inner div with height: 500px anymore */}
      <GoogleMap
        mapContainerStyle={{ width: '100%', height: '100%' }} // Map container fills the full screen div
        zoom={16}
        center={{ lat: 51.4985533, lng: -0.176324 }} // Default center - San Francisco
      >
        {geofences.map((geofence, index) => (
          <Circle
            key={index}
            center={geofence.center}
            radius={geofence.radius}
            options={{
              fillColor: '#FF0000',
              fillOpacity: 0.3,
              strokeColor: '#FF0000',
              strokeOpacity: 1,
              strokeWeight: 1,
            }}
          />
        ))}
      </GoogleMap>
    </div>
  );
};

export default GeofenceMap;