interface ElliptangleOptions {
    fillColor?: string;
    topLeftStroke?: string;
    bottomRightStroke?: string;
    strokeWidth?: number;
}

interface Point {
    x: number;
    y: number;
}

class ElliptangleRenderer {
    private canvas: HTMLCanvasElement;
    private ctx: CanvasRenderingContext2D;

    constructor(canvas: HTMLCanvasElement) {
        this.canvas = canvas;
        const context = canvas.getContext('2d');
        if (!context) {
            throw new Error('Unable to get 2D rendering context from canvas');
        }
        this.ctx = context;
    }

    drawElliptangleWithSeparateStrokes(
        x: number, 
        y: number, 
        w: number, 
        h: number, 
        curve: number, 
        options: ElliptangleOptions = {}
    ): void {
        const {
            fillColor = '#4CAF50',
            topLeftStroke = '#81C784',
            bottomRightStroke = '#2E7D32',
            strokeWidth = 3
        } = options;

        // First fill the shape
        this.ctx.beginPath();
        
        // Define the four corners
        const topLeft: Point = { x: x, y: y };
        const topRight: Point = { x: x + w, y: y };
        const bottomRight: Point = { x: x + w, y: y + h };
        const bottomLeft: Point = { x: x, y: y + h };
        
        // Start at top-left corner
        this.ctx.moveTo(topLeft.x, topLeft.y);
        
        // Top side (left to right) - curved outward
        this.ctx.quadraticCurveTo(
            x + w/2, y - curve,  // Control point (outward curve)
            topRight.x, topRight.y
        );
        
        // Right side (top to bottom) - curved outward
        this.ctx.quadraticCurveTo(
            x + w + curve, y + h/2,  // Control point (outward curve)
            bottomRight.x, bottomRight.y
        );
        
        // Bottom side (right to left) - curved outward
        this.ctx.quadraticCurveTo(
            x + w/2, y + h + curve,  // Control point (outward curve)
            bottomLeft.x, bottomLeft.y
        );
        
        // Left side (bottom to top) - curved outward
        this.ctx.quadraticCurveTo(
            x - curve, y + h/2,  // Control point (outward curve)
            topLeft.x, topLeft.y
        );
        
        this.ctx.closePath();
        
        // Fill the shape
        this.ctx.fillStyle = fillColor;
        this.ctx.fill();
        
        // Set line join and cap styles for clean connections
        this.ctx.lineJoin = 'round';
        this.ctx.lineCap = 'round';
        this.ctx.lineWidth = strokeWidth;
        
        // Draw top and left sides as one connected path
        this.ctx.strokeStyle = topLeftStroke;
        this.ctx.beginPath();
        this.ctx.moveTo(topLeft.x, topLeft.y);
        // Top side
        this.ctx.quadraticCurveTo(
            x + w/2, y - curve,
            topRight.x, topRight.y
        );
        this.ctx.stroke();
        
        this.ctx.beginPath();
        this.ctx.moveTo(bottomLeft.x, bottomLeft.y);
        // Left side
        this.ctx.quadraticCurveTo(
            x - curve, y + h/2,
            topLeft.x, topLeft.y
        );
        this.ctx.stroke();
        
        // Draw bottom and right sides as one connected path
        this.ctx.strokeStyle = bottomRightStroke;
        this.ctx.beginPath();
        this.ctx.moveTo(topRight.x, topRight.y);
        // Right side
        this.ctx.quadraticCurveTo(
            x + w + curve, y + h/2,
            bottomRight.x, bottomRight.y
        );
        this.ctx.stroke();
        
        this.ctx.beginPath();
        this.ctx.moveTo(bottomRight.x, bottomRight.y);
        // Bottom side
        this.ctx.quadraticCurveTo(
            x + w/2, y + h + curve,
            bottomLeft.x, bottomLeft.y
        );
        this.ctx.stroke();
    }

    clear(): void {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
    }
}

export { ElliptangleRenderer, type ElliptangleOptions, type Point };
