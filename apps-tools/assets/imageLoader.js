class ResponsiveImageLoader {
    constructor(imageElement) {
        this.img = imageElement;
        this.container = imageElement.parentElement;
        this.currentImage = null;
        this.path = imageElement.attributes.path.value
        this.sizes = imageElement.attributes.sizes.value.split(';').map(Number).sort((a, b) => a - b)
        this.currentImage = null;
        this.lastRequestedUrl = null;
        this.imageCache = new Map();
        this.init();
    }

    getImageUrlForWidth(width) {
        let selectedConfig = this.sizes[0];
        
        for (const size of this.sizes) {
            if (width >= size) {
                selectedConfig = size;
            } else {
                break;
            }
        }
        
        return this.path + "/" + selectedConfig + ".png";
    }

     async loadImage(url) {
        if (url === this.currentImage) return;
        
        try {           
            this.img.src = url;
            this.currentImage = url;
        } catch (error) {
            console.error('Error load image:', error);
        }
    }
    
    updateImage() {
        const containerWidth = this.container.clientWidth;
        const imageUrl = this.getImageUrlForWidth(containerWidth);
        this.loadImage(imageUrl);
    }
    
    init() {
        this.resizeObserver = new ResizeObserver(entries => {
            for (let entry of entries) {
                if (entry.target === this.container) {
                    this.updateImage();
                }
            }
        });
        
        this.resizeObserver.observe(this.container);
        this.updateImage();
    }
    
    destroy() {

        if (this.resizeObserver) {
            this.resizeObserver.disconnect();
        }
    }
}

const initImageLoaders = function(){
        const elements = document.querySelectorAll('img[path]');
        elements.forEach(element => {
            if (element.imageLoader === undefined){
                // console.log(element);
                element.imageLoader = new ResponsiveImageLoader(element);
            }
        });
}

$(function() {
    
    if (document.readyState !== 'loading') {
        console.log('DOM ready!');
        initImageLoaders();
    } else {
        document.addEventListener('DOMContentLoaded', function () {
            console.log('DOM loaded!');
            initImageLoaders();
        });
    }
});