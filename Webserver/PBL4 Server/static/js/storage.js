// Function to fetch image data with coordinates from the server
function fetchStorageData(page = 1) {
    fetch(`/api/get_storage_data?page=${page}`)
        .then(response => {
            if (!response.ok) {
                throw new Error('Network response was not ok');
            }
            return response.json();
        })
        .then(data => {
            if (data.image_info && data.image_info.length > 0) {
                updateStorageTable(data.image_info);
                updatePagination(data.page, data.total_pages);
            } else {
                console.error('No image data found.');
            }
        })
        .catch(error => console.error('Error fetching storage data:', error));
}

// Function to update the storage table dynamically
function updateStorageTable(imageInfo) {
    const gallery = document.querySelector('.image-gallery');
    gallery.innerHTML = ''; // Clear existing content

    imageInfo.forEach(item => {
        const imageItem = document.createElement('div');
        imageItem.className = 'image-item';

        imageItem.innerHTML = `
            <div class="image-info">
                <span>${item.id}</span> |
                <span>${item.datetime}</span> |
                <span>
                    <a href="/identification/${item.image_path}">
                        ${item.image_path}
                    </a>
                </span> |
                <span>${item.coordinates[0]}</span> |
                <span>${item.coordinates[1]}</span>
            </div>
        `;

        gallery.appendChild(imageItem);
    });
}

// Function to update pagination controls
function updatePagination(currentPage, totalPages) {
    const paginationContainer = document.querySelector('.pagination');
    paginationContainer.innerHTML = ''; // Clear existing content

    if (currentPage > 1) {
        const prevButton = document.createElement('a');
        prevButton.className = 'button';
        prevButton.href = `#`;
        prevButton.textContent = '« Trước';
        prevButton.addEventListener('click', () => fetchStorageData(currentPage - 1));
        paginationContainer.appendChild(prevButton);
    }

    const pageInfo = document.createElement('span');
    pageInfo.textContent = `Trang ${currentPage} của ${totalPages}`;
    paginationContainer.appendChild(pageInfo);

    if (currentPage < totalPages) {
        const nextButton = document.createElement('a');
        nextButton.className = 'button';
        nextButton.href = `#`;
        nextButton.textContent = 'Tiếp »';
        nextButton.addEventListener('click', () => fetchStorageData(currentPage + 1));
        paginationContainer.appendChild(nextButton);
    }
}

// Initial fetch when the page loads
fetchStorageData();
