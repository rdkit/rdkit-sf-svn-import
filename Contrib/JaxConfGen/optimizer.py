import numpy as np
import scipy.optimize

import jax
from jax.experimental import optimizers
import jax.lax

def minimize_scipy_bfgs(potential_fn, grad_fn, conf):


    N = conf.shape[0]
    D = conf.shape[1]

    def wrap_fun(x):
        x = np.reshape(x, (N, D))
        uu = potential_fn(x)
        gg = grad_fn(x)
        return uu, np.reshape(gg, (-1,))

    res = scipy.optimize.minimize(
        wrap_fun,
        conf,
        jac=True,
        method='BFGS',
        options={'disp': True}
    )

    return np.reshape(res.x, (N, D))

def minimize_jax_adam(potential_fn, grad_fn, conf):

    step_size = 2e-2
    num_epochs = 1000

    # (ytz): Unclear if Adam is actually a sane choice
    opt_init, opt_update, get_params = optimizers.adam(step_size)

    opt_state = opt_init(conf)

    def cond_fun(val):
        idx, _ = val
        return idx < num_epochs

    def body_fun(val):
        epoch, opt_state = val
        x_i = get_params(opt_state)
        dx = grad_fn(x_i)[0]
        opt_state = opt_update(epoch, dx, opt_state)
        return (epoch+1, opt_state)

    init_val = (0, opt_state)

    _, final_state = jax.lax.while_loop(cond_fun, body_fun, init_val)
    return get_params(final_state)

