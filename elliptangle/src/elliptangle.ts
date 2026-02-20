interface ElliptangleOptions {
    fillColor?: string;
    headerFillColor?: string;
    topLeftStroke?: string;
    bottomRightStroke?: string;
    strokeWidth?: number;
    text?: string;
    textColor?: string;
    fontFamily?: string;
    fontSizeRatio?: number; // Ratio of font size to height (default: 0.3)
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
            headerFillColor = '#81C784',
            topLeftStroke = '#81C784',
            bottomRightStroke = '#2E7D32',
            strokeWidth = 3,
            text = '',
            textColor = '#FFFFFF',
            fontFamily = 'Arial, sans-serif',
            fontSizeRatio = 0.3
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
        
        // Fill the main shape with the body color
        this.ctx.fillStyle = fillColor;
        this.ctx.fill();
        
        // Calculate header area height based on text
        const fontSize = Math.floor(h * fontSizeRatio);
        const headerHeight = text ? fontSize + (fontSize * 0.1) : 0; // Text height + minimal padding for tighter fit
        
        // If there's text, create a header section with different color
        if (text && headerHeight > 0) {
            this.drawHeaderSection(x, y, w, h, curve, headerHeight, headerFillColor);
        }
        
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
        
        // Draw text if provided
        if (text) {
            this.drawText(x, y, w, h, curve, text, textColor, fontFamily, fontSizeRatio);
        }
    }

    private drawText(
        x: number, 
        y: number, 
        w: number, 
        h: number, 
        curve: number,
        text: string, 
        textColor: string, 
        fontFamily: string, 
        fontSizeRatio: number
    ): void {
        // Calculate font size based on height
        const fontSize = Math.floor(h * fontSizeRatio);
        
        // Set font properties
        this.ctx.fillStyle = textColor;
        this.ctx.font = `${fontSize}px ${fontFamily}`;
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'top';
        
        // Calculate center X position and safe Y position
        const centerX = x + w / 2;
        // Calculate safe Y position that avoids the curved area
        const curveDepth = this.calculateCurveDepthAtPosition(x, y, w, h, curve);
        const safeTopY = y + Math.max(curveDepth * 0.7, fontSize * 0.05); // More aggressive positioning - 70% of curve depth or minimal padding
        
        // Measure text to ensure it fits
        const textMetrics = this.ctx.measureText(text);
        const textWidth = textMetrics.width;
        
        // If text is too wide, reduce font size
        if (textWidth > w * 0.8) { // Leave 20% padding
            const adjustedFontSize = Math.floor((w * 0.8 / textWidth) * fontSize);
            this.ctx.font = `${adjustedFontSize}px ${fontFamily}`;
        }
        
        // Draw the text
        this.ctx.fillText(text, centerX, safeTopY);
    }

    private calculateCurveDepthAtPosition(x: number, y: number, w: number, h: number, curve: number): number {
        // Calculate how deep the curve extends at the text position
        // For the top curve, the maximum depth is at the center (x + w/2)
        // We need to find the Y position where the curve would be at the text X position
        
        // At the center of the elliptangle width, the curve extends 'curve' pixels upward
        // We want to position text so it's just below this curve
        const textWidthArea = w * 0.8; // Text area is 80% of width
        const textStartX = x + w * 0.1; // Text starts at 10% from left
        const textEndX = x + w * 0.9; // Text ends at 90% from right
        
        // Calculate curve depth at the text boundaries
        // The curve is a quadratic, so we need to find the Y offset at these X positions
        const normalizedStartX = (textStartX - x) / w; // 0 to 1
        const normalizedEndX = (textEndX - x) / w; // 0 to 1
        
        // For a quadratic curve, the depth varies quadratically from edges to center
        // At edges (0, 1) depth is 0, at center (0.5) depth is maximum (curve)
        const depthAtStart = 4 * curve * normalizedStartX * (1 - normalizedStartX);
        const depthAtEnd = 4 * curve * normalizedEndX * (1 - normalizedEndX);
        
        // Use the maximum depth in the text area
        const maxDepth = Math.max(depthAtStart, depthAtEnd);
        
        return maxDepth;
    }

    private drawHeaderSection(
        x: number, 
        y: number, 
        w: number, 
        h: number, 
        curve: number, 
        headerHeight: number, 
        headerFillColor: string
    ): void {
        // Calculate the safe text position and actual header height needed
        const fontSize = Math.floor(h * 0.3); // Use the same calculation as in drawText
        const curveDepth = this.calculateCurveDepthAtPosition(x, y, w, h, curve);
        const safeTopY = y + Math.max(curveDepth * 0.7, fontSize * 0.05); // More aggressive positioning
        const actualHeaderHeight = (safeTopY - y) + fontSize + (fontSize * 0.05); // Tighter spacing
        
        // Calculate the horizontal line position (just below the text area)
        const lineY = y + actualHeaderHeight;
        
        // Create a clipping path for the header area
        this.ctx.save();
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
            x + w/2, y - curve,
            topRight.x, topRight.y
        );
        
        // Right side (top to bottom) - curved outward, but only to lineY
        const rightCurvePoint = this.getPointOnCurve(
            topRight.x, topRight.y,
            x + w + curve, y + h/2,
            bottomRight.x, bottomRight.y,
            Math.min((lineY - y) / h, 1.0) // Ensure t doesn't exceed 1
        );
        
        if ((lineY - y) / h < 1.0) {
            this.ctx.quadraticCurveTo(
                x + w + curve, y + h/2,
                rightCurvePoint.x, rightCurvePoint.y
            );
            
            // Horizontal line across
            const leftCurvePoint = this.getPointOnCurve(
                topLeft.x, topLeft.y,
                x - curve, y + h/2,
                bottomLeft.x, bottomLeft.y,
                (lineY - y) / h
            );
            
            this.ctx.lineTo(leftCurvePoint.x, leftCurvePoint.y);
            
            // Left side back to top
            this.ctx.quadraticCurveTo(
                x - curve, y + h/2,
                topLeft.x, topLeft.y
            );
        } else {
            // If header extends beyond the shape, just draw the full shape
            this.ctx.quadraticCurveTo(
                x + w + curve, y + h/2,
                bottomRight.x, bottomRight.y
            );
            this.ctx.quadraticCurveTo(
                x + w/2, y + h + curve,
                bottomLeft.x, bottomLeft.y
            );
            this.ctx.quadraticCurveTo(
                x - curve, y + h/2,
                topLeft.x, topLeft.y
            );
        }
        
        this.ctx.clip();
        
        // Fill the header area
        this.ctx.fillStyle = headerFillColor;
        this.ctx.fillRect(x - curve, y - curve, w + 2 * curve, actualHeaderHeight + curve);
        
        this.ctx.restore();
    }

    private getPointOnCurve(
        startX: number, 
        startY: number, 
        controlX: number, 
        controlY: number, 
        endX: number, 
        endY: number, 
        t: number
    ): Point {
        // Quadratic Bezier curve formula: P(t) = (1-t)²P₀ + 2(1-t)tP₁ + t²P₂
        const oneMinusT = 1 - t;
        const x = oneMinusT * oneMinusT * startX + 2 * oneMinusT * t * controlX + t * t * endX;
        const y = oneMinusT * oneMinusT * startY + 2 * oneMinusT * t * controlY + t * t * endY;
        return { x, y };
    }

    clear(): void {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
    }
}

export { ElliptangleRenderer, type ElliptangleOptions, type Point };
