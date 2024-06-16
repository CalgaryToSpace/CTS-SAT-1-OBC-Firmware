document.addEventListener("DOMContentLoaded", function() {
    function setupScrollListener() {
        let container = document.getElementById("rx-tx-log-container");
        if (container) {
            // When the user scrolls down 20px from the top of the rx-tx-log-container, show the button
            container.onscroll = function() { scrollFunction() };

            function scrollFunction() {
                // console.log("Scrolling: " + container.scrollTop);
                let jump_button = document.getElementById("scroll-to-bottom-button");
                if (container.scrollTop < -20) {
                    jump_button.style.display = "block";
                } else {
                    jump_button.style.display = "none";
                }
            }

            // When the user clicks on the button, scroll to the top of the rx-tx-log-container
            document.getElementById("scroll-to-bottom-button").onclick = function() {
                container.scrollTop = 0;
            }
        } else {
            // Retry after a short delay if the container is not yet available
            setTimeout(setupScrollListener, 100);
        }
    }

    setupScrollListener();
});
