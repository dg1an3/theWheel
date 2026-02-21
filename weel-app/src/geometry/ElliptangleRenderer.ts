import { Vector2D } from './Vector2D';

export interface ElliptangleOptions {
    fillColor?: string;
    headerFillColor?: string;
    topLeftStroke?: string;
    bottomRightStroke?: string;
    strokeWidth?: number;
    text?: string;
    textColor?: string;
    fontFamily?: string;
    fontSizeRatio?: number;
}

export class ElliptangleRenderer {
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

        const topLeft = new Vector2D(x, y);
        const topRight = new Vector2D(x + w, y);
        const bottomRight = new Vector2D(x + w, y + h);
        const bottomLeft = new Vector2D(x, y + h);

        // Fill the shape
        this.ctx.beginPath();
        this.ctx.moveTo(topLeft.x, topLeft.y);
        this.ctx.quadraticCurveTo(x + w / 2, y - curve, topRight.x, topRight.y);
        this.ctx.quadraticCurveTo(x + w + curve, y + h / 2, bottomRight.x, bottomRight.y);
        this.ctx.quadraticCurveTo(x + w / 2, y + h + curve, bottomLeft.x, bottomLeft.y);
        this.ctx.quadraticCurveTo(x - curve, y + h / 2, topLeft.x, topLeft.y);
        this.ctx.closePath();
        this.ctx.fillStyle = fillColor;
        this.ctx.fill();

        // Header section
        const fontSize = Math.floor(h * fontSizeRatio);
        const headerHeight = text ? fontSize + (fontSize * 0.1) : 0;
        if (text && headerHeight > 0) {
            this.drawHeaderSection(x, y, w, h, curve, headerHeight, headerFillColor);
        }

        // Stroke styles
        this.ctx.lineJoin = 'round';
        this.ctx.lineCap = 'round';
        this.ctx.lineWidth = strokeWidth;

        // Top side stroke
        this.ctx.strokeStyle = topLeftStroke;
        this.ctx.beginPath();
        this.ctx.moveTo(topLeft.x, topLeft.y);
        this.ctx.quadraticCurveTo(x + w / 2, y - curve, topRight.x, topRight.y);
        this.ctx.stroke();

        // Left side stroke
        this.ctx.beginPath();
        this.ctx.moveTo(bottomLeft.x, bottomLeft.y);
        this.ctx.quadraticCurveTo(x - curve, y + h / 2, topLeft.x, topLeft.y);
        this.ctx.stroke();

        // Right side stroke
        this.ctx.strokeStyle = bottomRightStroke;
        this.ctx.beginPath();
        this.ctx.moveTo(topRight.x, topRight.y);
        this.ctx.quadraticCurveTo(x + w + curve, y + h / 2, bottomRight.x, bottomRight.y);
        this.ctx.stroke();

        // Bottom side stroke
        this.ctx.beginPath();
        this.ctx.moveTo(bottomRight.x, bottomRight.y);
        this.ctx.quadraticCurveTo(x + w / 2, y + h + curve, bottomLeft.x, bottomLeft.y);
        this.ctx.stroke();

        // Draw text
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
        const fontSize = Math.floor(h * fontSizeRatio);
        this.ctx.fillStyle = textColor;
        this.ctx.font = `${fontSize}px ${fontFamily}`;
        this.ctx.textAlign = 'center';
        this.ctx.textBaseline = 'top';

        const centerX = x + w / 2;
        const curveDepth = this.calculateCurveDepthAtPosition(x, y, w, h, curve);
        const safeTopY = y + Math.max(curveDepth * 0.7, fontSize * 0.05);

        const textMetrics = this.ctx.measureText(text);
        if (textMetrics.width > w * 0.8) {
            const adjustedFontSize = Math.floor((w * 0.8 / textMetrics.width) * fontSize);
            this.ctx.font = `${adjustedFontSize}px ${fontFamily}`;
        }

        this.ctx.fillText(text, centerX, safeTopY);
    }

    private calculateCurveDepthAtPosition(
        _x: number,
        _y: number,
        w: number,
        _h: number,
        curve: number
    ): number {
        const normalizedStartX = 0.1;
        const normalizedEndX = 0.9;
        const depthAtStart = 4 * curve * normalizedStartX * (1 - normalizedStartX);
        const depthAtEnd = 4 * curve * normalizedEndX * (1 - normalizedEndX);
        return Math.max(depthAtStart, depthAtEnd);
    }

    private drawHeaderSection(
        x: number,
        y: number,
        w: number,
        h: number,
        curve: number,
        _headerHeight: number,
        headerFillColor: string
    ): void {
        const fontSize = Math.floor(h * 0.3);
        const curveDepth = this.calculateCurveDepthAtPosition(x, y, w, h, curve);
        const safeTopY = y + Math.max(curveDepth * 0.7, fontSize * 0.05);
        const actualHeaderHeight = (safeTopY - y) + fontSize + (fontSize * 0.05);
        const lineY = y + actualHeaderHeight;

        const topLeft = new Vector2D(x, y);
        const topRight = new Vector2D(x + w, y);
        const bottomRight = new Vector2D(x + w, y + h);
        const bottomLeft = new Vector2D(x, y + h);

        this.ctx.save();
        this.ctx.beginPath();
        this.ctx.moveTo(topLeft.x, topLeft.y);
        this.ctx.quadraticCurveTo(x + w / 2, y - curve, topRight.x, topRight.y);

        const t = Math.min((lineY - y) / h, 1.0);
        if (t < 1.0) {
            const rightCurvePoint = this.getPointOnCurve(
                topRight.x, topRight.y,
                x + w + curve, y + h / 2,
                bottomRight.x, bottomRight.y, t
            );
            this.ctx.quadraticCurveTo(
                x + w + curve, y + h / 2,
                rightCurvePoint.x, rightCurvePoint.y
            );

            const leftCurvePoint = this.getPointOnCurve(
                topLeft.x, topLeft.y,
                x - curve, y + h / 2,
                bottomLeft.x, bottomLeft.y, t
            );
            this.ctx.lineTo(leftCurvePoint.x, leftCurvePoint.y);
            this.ctx.quadraticCurveTo(x - curve, y + h / 2, topLeft.x, topLeft.y);
        } else {
            this.ctx.quadraticCurveTo(x + w + curve, y + h / 2, bottomRight.x, bottomRight.y);
            this.ctx.quadraticCurveTo(x + w / 2, y + h + curve, bottomLeft.x, bottomLeft.y);
            this.ctx.quadraticCurveTo(x - curve, y + h / 2, topLeft.x, topLeft.y);
        }

        this.ctx.clip();
        this.ctx.fillStyle = headerFillColor;
        this.ctx.fillRect(x - curve, y - curve, w + 2 * curve, actualHeaderHeight + curve);
        this.ctx.restore();
    }

    private getPointOnCurve(
        startX: number, startY: number,
        controlX: number, controlY: number,
        endX: number, endY: number,
        t: number
    ): Vector2D {
        const oneMinusT = 1 - t;
        return new Vector2D(
            oneMinusT * oneMinusT * startX + 2 * oneMinusT * t * controlX + t * t * endX,
            oneMinusT * oneMinusT * startY + 2 * oneMinusT * t * controlY + t * t * endY
        );
    }

    clear(): void {
        this.ctx.clearRect(0, 0, this.canvas.width, this.canvas.height);
    }
}
