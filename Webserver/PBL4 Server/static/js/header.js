// Wait for the DOM to fully load
document.addEventListener("DOMContentLoaded", () => {
    // Select all navigation links
    const navbarLinks = document.querySelectorAll('.navbar a');

    // Add click event listener to each link
    navbarLinks.forEach(link => {
        link.addEventListener('click', (event) => {
            const href = link.getAttribute('href');
            event.preventDefault(); // Prevent default anchor click behavior

            // Check if the link is for a section on the same page
            if (href.startsWith('#')) {
                const targetId = href.substring(1);
                const targetSection = document.getElementById(targetId);
                
                // Scroll to the target section smoothly
                if (targetSection) {
                    targetSection.scrollIntoView({ behavior: 'smooth', block: 'start' });
                }
            } else {
                // If it's a link to another HTML file, open it in the iframe
                window.open(href, "sidebar");
            }
        });
    });
});