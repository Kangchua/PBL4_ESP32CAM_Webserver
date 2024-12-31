var map = L.map('map').setView([16.0544, 108.2022], 13); // Centered on Đà Nẵng

// Add a tile layer to the map
L.tileLayer('https://{s}.tile.openstreetmap.org/{z}/{x}/{y}.png', {
    attribution: '&copy; <a href="https://www.openstreetmap.org/copyright">OpenStreetMap</a> contributors'
}).addTo(map);

// Placeholder for coordinates from your identification.js
var coordinates = [16.0544, 108.2022]; // Example coordinates
var marker = L.marker(coordinates).addTo(map)
    .bindPopup('Detected coordinates')
    .openPopup();

// Update coordinates display
document.getElementById('coordinates').textContent = `X: ${coordinates[0]}, Y: ${coordinates[1]}`;

function updateImage() {
    fetch('/identification')
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            const imageElement = document.getElementById('identification-image');
            if (data.image_path && data.image_path !== imageElement.src) {
                imageElement.src = data.image_path + '?' + new Date().getTime(); // Thêm dấu hỏi để tránh cache
            }
        })
        .catch(error => console.error('Error fetching new image:', error));
}

const intervalId = setInterval(updateImage, 30000);

// Dừng cập nhật khi người dùng rời khỏi trang
window.addEventListener('beforeunload', () => {
    clearInterval(intervalId);
});