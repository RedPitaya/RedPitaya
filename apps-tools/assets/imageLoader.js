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

const initImageLoaders = function(force){
        const elements = document.querySelectorAll('img[path]');
        elements.forEach(element => {
            if (element.imageLoader === undefined || force){
                // console.log(element);
                if (element.imageLoader !== undefined)
                    element.imageLoader.destroy()
                    delete element.imageLoader
                element.imageLoader = new ResponsiveImageLoader(element);
            }else{
                element.imageLoader.updateImage()
            }
        });
}


const setBoardPinOut = function(model) {
//  STEM_125_10_v1_0            = 0,
//  STEM_125_14_v1_0            = 1,
//  STEM_125_14_v1_1            = 2,
//  STEM_125_14_LN_v1_1         = 5,
//  STEM_125_14_LN_BO_v1_1      = 17,
//  STEM_125_14_LN_CE1_v1_1     = 18,
//  STEM_125_14_LN_CE2_v1_1     = 19,

    if (model == 0 || model == 1 || model == 2 || model == 5 || model == 17 || model == 18 || model == 19) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_125_14')
        $('#pinoutId').attr('sizes','2560')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 1.66, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    }

//  STEM_125_14_Z7020_v1_0      = 6,
//  STEM_125_14_Z7020_LN_v1_1   = 7,

    if (model == 6 || model == 7) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_125_14_Z7020')
        $('#pinoutId').attr('sizes','2560')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 1.66, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    }

//  STEM_122_16SDR_v1_0         = 3,
//  STEM_122_16SDR_v1_1         = 4,

    if (model == 3 || model == 4) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_122_16')
        $('#pinoutId').attr('sizes','2560')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 1.66, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    };

//  STEM_250_12_v1_0            = 11,
//  STEM_250_12_v1_1            = 12,
//  STEM_250_12_v1_2            = 13,
//  STEM_250_12_120             = 14,
//  STEM_250_12_v1_2a           = 15,
//  STEM_250_12_v1_2b           = 16,

    if (model == 11 || model == 12 || model == 13 || model == 14 || model == 15 || model == 16) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_250_12')
        $('#pinoutId').attr('sizes','2560')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 1.66, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    };

//  STEM_125_14_Z7020_4IN_v1_0  = 8,
//  STEM_125_14_Z7020_4IN_v1_2  = 9,
//  STEM_125_14_Z7020_4IN_v1_3  = 10,
//  STEM_125_14_Z7020_4IN_BO_v1_3  = 30,

    if (model == 8 || model == 9 || model == 10 || model == 30) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_125_4ch')
        $('#pinoutId').attr('sizes','2560')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 1.66, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    };

//  STEM_125_14_v2_0            = 20,

    if (model == 20) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_125_14_v2_0')
        $('#pinoutId').attr('sizes','2560')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 1.66, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    };

//  STEM_125_14_Pro_v2_0        = 21,

    if (model == 21) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_125_14_v2_0_pro')
        $('#pinoutId').attr('sizes','4000')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 2.61, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    };

//  STEM_125_14_Z7020_Pro_v2_0  = 22,
//  STEM_125_14_Z7020_Ind_v2_0  = 23,
//  STEM_125_14_Z7020_Pro_v1_0  = 24,

    if (model == 22 || model == 23 || model == 24) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_125_14_v2_0_pro_Z7020')
        $('#pinoutId').attr('sizes','4000')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 2.61, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    };


//  STEM_125_14_Z7020_LL_v1_1   = 25,
//  STEM_125_14_Z7020_LL_v1_2   = 27,
//  STEM_125_14_Z7020_TI_v1_3   = 28,
//  STEM_65_16_Z7020_LL_v1_1    = 26,
//  STEM_65_16_Z7020_TI_v1_3    = 29,

    if (model == 25 || model == 27 || model == 28 || model == 26 || model == 29) {
        $('#pinoutId').attr('path','../assets/images/pack/pinout_125_14_Z7020')
        $('#pinoutId').attr('sizes','2560')
        var calcW = Math.min(document.body.clientHeight / 2.0 * 1.66, document.body.clientWidth * 0.8)
        $('#pinoutDialogId').css('width',Math.round(calcW)+'px')
    };

    initImageLoaders(true);
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