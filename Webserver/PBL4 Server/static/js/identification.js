// Function to fetch and update identification data
function updateIdentification() {
    fetch('/api/get_latest_coordinates')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            if (data.latitude && data.longitude) {
                const coordinates = [data.latitude, data.longitude];
                map.setView(coordinates, 13); // Adjust zoom level if needed
                marker.setLatLng(coordinates)
                    .bindPopup(`Tọa độ: X: ${data.latitude}, Y: ${data.longitude}`)
                    .openPopup();

                // Update the coordinates display
                document.getElementById('coordinates').textContent = `X: ${data.latitude}, Y: ${data.longitude}`;
            } else {
                console.error('No coordinates found.');
            }
        })
        .catch(error => console.error('Error fetching coordinates:', error));
}

// Update identification data every 5 seconds
const intervalId = setInterval(updateIdentification, 5000);

// Clear interval when the page is unloaded
window.addEventListener('beforeunload', () => clearInterval(intervalId));

// Initial fetch on page load
updateIdentification();
