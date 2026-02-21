import React from 'react';
import { ElliptangleOptions } from '../../geometry';

interface ElliptangleControlsProps {
    activation: number;
    curve: number;
    shapeWidth: number;
    shapeHeight: number;
    options: ElliptangleOptions;
    onActivationChange: (value: number) => void;
    onCurveChange: (value: number) => void;
    onWidthChange: (value: number) => void;
    onHeightChange: (value: number) => void;
    onOptionsChange: (options: Partial<ElliptangleOptions>) => void;
}

const ElliptangleControls: React.FC<ElliptangleControlsProps> = ({
    activation,
    curve,
    shapeWidth,
    shapeHeight,
    options,
    onActivationChange,
    onCurveChange,
    onWidthChange,
    onHeightChange,
    onOptionsChange
}) => {
    return (
        <div className="elliptangle-controls">
            <div className="control-group">
                <label>Activation:</label>
                <input type="range" min="0" max="100" value={activation}
                    onChange={e => onActivationChange(Number(e.target.value))} />
                <span>{activation}</span>
            </div>
            <div className="control-group">
                <label>Curve Amount:</label>
                <input type="range" min="0" max="50" value={curve}
                    onChange={e => onCurveChange(Number(e.target.value))} />
                <span>{curve}</span>
            </div>
            <div className="control-group">
                <label>Width:</label>
                <input type="range" min="100" max="400" value={shapeWidth}
                    onChange={e => onWidthChange(Number(e.target.value))} />
                <span>{shapeWidth}</span>
            </div>
            <div className="control-group">
                <label>Height:</label>
                <input type="range" min="80" max="300" value={shapeHeight}
                    onChange={e => onHeightChange(Number(e.target.value))} />
                <span>{shapeHeight}</span>
            </div>
            <div className="control-group">
                <label>Fill Color:</label>
                <input type="color" value={options.fillColor || '#4CAF50'}
                    onChange={e => onOptionsChange({ fillColor: e.target.value })} />
            </div>
            <div className="control-group">
                <label>Header Fill Color:</label>
                <input type="color" value={options.headerFillColor || '#81C784'}
                    onChange={e => onOptionsChange({ headerFillColor: e.target.value })} />
            </div>
            <div className="control-group">
                <label>Top/Left Stroke:</label>
                <input type="color" value={options.topLeftStroke || '#81C784'}
                    onChange={e => onOptionsChange({ topLeftStroke: e.target.value })} />
            </div>
            <div className="control-group">
                <label>Bottom/Right Stroke:</label>
                <input type="color" value={options.bottomRightStroke || '#2E7D32'}
                    onChange={e => onOptionsChange({ bottomRightStroke: e.target.value })} />
            </div>
            <div className="control-group">
                <label>Stroke Width:</label>
                <input type="range" min="1" max="10" value={options.strokeWidth || 3}
                    onChange={e => onOptionsChange({ strokeWidth: Number(e.target.value) })} />
                <span>{options.strokeWidth || 3}</span>
            </div>
            <div className="control-group">
                <label>Text:</label>
                <input type="text" value={options.text || ''} placeholder="Enter text..."
                    onChange={e => onOptionsChange({ text: e.target.value })} />
            </div>
            <div className="control-group">
                <label>Text Color:</label>
                <input type="color" value={options.textColor || '#FFFFFF'}
                    onChange={e => onOptionsChange({ textColor: e.target.value })} />
            </div>
            <div className="control-group">
                <label>Font Size Ratio:</label>
                <input type="range" min="0.1" max="0.8" step="0.05"
                    value={options.fontSizeRatio || 0.3}
                    onChange={e => onOptionsChange({ fontSizeRatio: Number(e.target.value) })} />
                <span>{(options.fontSizeRatio || 0.3).toFixed(2)}</span>
            </div>
        </div>
    );
};

export default ElliptangleControls;
