var map = L.map('map').setView([16.0544, 108.2022], 13); // Initial map centered on Đà Nẵng

// Add a tile layer to the map
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(map);

// Placeholder for the marker (will be updated dynamically)
var marker = L.marker([0, 0]).addTo(map);

// Function to fetch data from the server
function updateCoordinates() {
    fetch('/api/get_latest_coordinates') // Update with the correct API endpoint
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            if (data.latitude && data.longitude) {
                // Update the map view and marker
                const coordinates = [data.latitude, data.longitude];
                map.setView(coordinates, 13); // Adjust zoom level if needed
                marker.setLatLng(coordinates).bindPopup('Detected coordinates').openPopup();

                // Update the coordinates display
                document.getElementById('coordinates').textContent = `X: ${data.latitude}, Y: ${data.longitude}`;
            } else {
                console.error('Coordinates are missing in the response.');
            }
        })
        .catch(error => console.error('Error fetching coordinates:', error));
}

// Update the map and coordinates every 5 seconds
const intervalId = setInterval(updateCoordinates, 5000);

// Stop updates when the user leaves the page
window.addEventListener('beforeunload', () => {
    clearInterval(intervalId);
});

// Initial fetch when the page loads
updateCoordinates();
